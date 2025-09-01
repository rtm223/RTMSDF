// Copyright (c) Richard Meredith AB. All Rights Reserved

#include "RTMSDF_SVGFactory.h"
#include "MSDF/RTMSDF_MSDFGenerationHelpers.h"
#include "Importer/Common/RTMSDFTextureSettingsCache.h"
#include "Generation/SVG/RTMSDF_SVGGenerationAssetData.h"
#include "Generation/SVG/RTMSDF_SVGGenerationSettings.h"
#include "ChlumskyMSDFGen/Public/Core/msdfgen.h"
#include "ChlumskyMSDFGen/Public/Ext/import-svg.h"
#include "Settings/RTMSDF_ProjectSettings.h"
#include "Core/SDFTransformation.h"
#include "Curves/CurveLinearColorAtlas.h"
#include "EditorFramework/AssetImportData.h"
#include "Engine/Texture2DArray.h"
#include "Module/RTMSDFEditor.h"
#include "Core/Vector2.hpp"
#include "MSDF/RTMSDF_MSDFTextureHelpers.h"

#if ENGINE_MAJOR_VERSION >=5 && ENGINE_MINOR_VERSION >=2
#include "TextureReferenceResolver.h"
#endif

URTMSDF_SVGFactory::URTMSDF_SVGFactory()
{
	bCreateNew = false;
	bEditorImport = true;

	SupportedClass = UTexture2D::StaticClass();
	Formats.Add("svg;Scalable Vector Graphics");
}

bool URTMSDF_SVGFactory::IsAutomatedImport() const
{
	return Super::IsAutomatedImport() || IsAutomatedReimport();
}

bool URTMSDF_SVGFactory::FactoryCanImport(const FString& filename)
{
	if(const auto* settings = GetDefault<URTMSDF_ProjectSettings>())
	{
		const auto& suffix = settings->SVGFilenameSuffix;
		if(suffix.Len() == 0 || filename.EndsWith(suffix))
			return true;
	}
	return false;
}

UObject* URTMSDF_SVGFactory::FactoryCreateBinary(UClass* inClass, UObject* inParent, FName inName, EObjectFlags flags, UObject* context, const TCHAR* type, const uint8*& buffer, const uint8* bufferEnd, FFeedbackContext* warn)
{
	const uint64 cyclesStart = FPlatformTime::Cycles();

	using namespace msdfgen;
	using namespace RTM::SDF::MSDFGenerationHelpers;
	using namespace RTM::SDF::MSDFTextureHelpers;

	GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPreImport(this, inClass, inParent, inName, type);

	auto existingTexture = FindObject<UTexture2D>(inParent, *inName.ToString());

	if(existingTexture)
	{
		existingTexture->UpdateResource();
		existingTexture->WaitForPendingInitOrStreaming();
	}
	else if(!FactoryCanImport(inName.ToString()))
	{
		// Unreal will still try to use us if if we tell it not to, if there are no back up factories to use, so we double check here
		UE_LOG(RTMSDFEditor, Error, TEXT("Import for %s failed - Filename does not match the correct format"), *inName.ToString());
		return nullptr;
	}

	FTextureReferenceReplacer RefReplacer(existingTexture);
	FRTMSDFTextureSettingsCache textureSettings(existingTexture);
	FRTMSDF_SVGGenerationSettings importerSettings;
	if(const auto* previousSettings = existingTexture ? existingTexture->GetAssetUserData<URTMSDF_SVGGenerationAssetData>() : nullptr)
	{
		importerSettings = previousSettings->GenerationSettings;
	}
	else if(const auto* defaultConfig = GetDefault<URTMSDF_ProjectSettings>())
	{
		importerSettings = defaultConfig->DefaultSVGImportSettings;
		importerSettings.bIsInProjectSettings = false;
		textureSettings.LODGroup = defaultConfig->SVGTextureGroup;
	}

	Shape shape;
	Shape::Bounds svgBounds;
	if(!CreateShape(buffer, bufferEnd, shape, svgBounds))
	{
		UE_LOG(RTMSDFEditor, Error, TEXT("Import for %s failed - unable to create Shape"), *inName.ToString());
		GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, nullptr);
		return nullptr;
	}

	// TODO - test with a bounds that goes negative somehow
	const Vector2 svgSize(svgBounds.r, svgBounds.t);

	shape.normalize();
	if(!ensureAlwaysMsgf(shape.validate(), TEXT("Failed to validate MSDF shape")))
		return nullptr;

	if(importerSettings.Format == ERTMSDF_SDFFormat::Multichannel || importerSettings.Format == ERTMSDF_SDFFormat::MultichannelPlusAlpha)
		DoEdgeColoring(shape, importerSettings.EdgeColoringMode, FMath::DegreesToRadians(importerSettings.MaxCornerAngle), importerSettings.EdgeColoringSeed);

	UTexture2D* texture = nullptr;
	if(auto newObject = CreateOrOverwriteAsset(inClass, inParent, inName, flags))
		texture = CastChecked<UTexture2D>(newObject);

	// NOTE: existingTexture will now point to the new texture - don't try to use its values after this

	if(!texture)
	{
		if(existingTexture)
			existingTexture->UpdateResource();

		UE_LOG(RTMSDFEditor, Error, TEXT("Import for %s failed - unable to create Texture"), *inName.ToString());
		GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, nullptr);
		return nullptr;
	}

	Vector2 sdfSize;
	const double range = importerSettings.GetAbsoluteRange({svgSize.x, svgSize.y});
	const SDFTransformation sdfTransformation = CalculateTransformation(svgSize, importerSettings.TextureSize, importerSettings.bScaleToFitDistance, range, sdfSize);
	MSDFGeneratorConfig generatorConfig;
	generatorConfig.overlapSupport = true;

	ApplyErrorCorrectionModeTo(generatorConfig.errorCorrection, importerSettings.ErrorCorrectionMode);
	PopulateSDFTextureSourceData(importerSettings.Format, generatorConfig, sdfSize, shape, sdfTransformation, importerSettings.bInvertDistance, texture);

	UpdateNewTextureSettings(texture, textureSettings, importerSettings.Format);

	auto* importAssetData = texture->GetAssetUserData<URTMSDF_SVGGenerationAssetData>();
	if(!importAssetData)
	{
		importAssetData = NewObject<URTMSDF_SVGGenerationAssetData>(texture, NAME_None, flags);
		texture->AddAssetUserData(importAssetData);
	}

	importAssetData->GenerationSettings = importerSettings;
	importAssetData->UVRange = importerSettings.GetNormalizedRange({svgSize.x, svgSize.y});
	importAssetData->SourceDimensions = {static_cast<int>(svgSize.x), static_cast<int>(svgSize.y)};

	texture->bHasBeenPaintedInEditor = false;

	RefReplacer.Replace(texture);

	texture->AssetImportData->Update(CurrentFilename, FileHash.IsValid() ? &FileHash : nullptr);
	GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, texture);
	texture->PostEditChange();

	for(TObjectIterator<UTexture2DArray> it; it; ++it)
	{
		UTexture2DArray* textureArray = *it;
		if(textureArray)
		{
			for(int32 SourceIndex = 0; SourceIndex < textureArray->SourceTextures.Num(); ++SourceIndex)
			{
				if(textureArray->SourceTextures[SourceIndex] == texture)
				{
					// Update the entire texture array.
					textureArray->UpdateSourceFromSourceTextures(false);
					break;
				}
			}
		}
	}
	const uint64 cyclesEnd = FPlatformTime::Cycles();
	UE_LOG(RTMSDFEditor, Log, TEXT("Import Complete - %.2f miliseconds"), FPlatformTime::ToMilliseconds(cyclesEnd-cyclesStart));

	return texture;
}

int32 URTMSDF_SVGFactory::GetPriority() const
{
	return INT32_MAX;
}

bool URTMSDF_SVGFactory::CanReimport(UObject* obj, TArray<FString>& outFilenames)
{
	auto* tex = Cast<UTexture2D>(obj);
	if(tex && !tex->IsA<UCurveLinearColorAtlas>() && tex->GetAssetUserData<URTMSDF_SVGGenerationAssetData>())
	{
		tex->AssetImportData->ExtractFilenames(outFilenames);
		return true;
	}
	return false;
}

void URTMSDF_SVGFactory::SetReimportPaths(UObject* obj, const TArray<FString>& newReimportPaths)
{
	UTexture* tex = Cast<UTexture2D>(obj);
	if(tex && ensure(newReimportPaths.Num() == 1))
		tex->AssetImportData->UpdateFilenameOnly(newReimportPaths[0]);
}

EReimportResult::Type URTMSDF_SVGFactory::Reimport(UObject* obj)
{
	auto* texture = Cast<UTexture2D>(obj);
	if(!ensure(texture))
		return EReimportResult::Failed;

	const FString textureName = texture->GetName();
	const FString resolvedSourceFilePath = texture->AssetImportData->GetFirstFilename();

	if(!resolvedSourceFilePath.Len())
	{
		UE_LOG(RTMSDFEditor, Error, TEXT("Cannot reimport %s: texture resource does not have path stored."), *textureName);
		return EReimportResult::Failed;
	}
	if(IFileManager::Get().FileSize(*resolvedSourceFilePath) == INDEX_NONE)
	{
		UE_LOG(RTMSDFEditor, Warning, TEXT("Cannot reimport %s: source file [%s] cannot be found."), *textureName, *resolvedSourceFilePath);
		return EReimportResult::Failed;
	}

	UE_LOG(RTMSDFEditor, Log, TEXT("Performing atomic reimport of %s [%s]"), *textureName, *resolvedSourceFilePath);

	// record settings from texture - saved as member variables in the class because unreal is icky

	bool outCancelled = false;
	if(ImportObject(texture->GetClass(), texture->GetOuter(), *textureName, RF_Public | RF_Standalone, resolvedSourceFilePath, nullptr, outCancelled))
	{
		if(auto outer = texture->GetOuter())
			outer->MarkPackageDirty();
		else
			texture->MarkPackageDirty();

		texture->AssetImportData->Update(resolvedSourceFilePath);

		return EReimportResult::Succeeded;
	}
	else if(outCancelled)
	{
		UE_LOG(RTMSDFEditor, Warning, TEXT("import of %s canceled"), *textureName);
		return EReimportResult::Cancelled;
	}

	UE_LOG(RTMSDFEditor, Warning, TEXT("import of %s failed"), *textureName);
	return EReimportResult::Failed;
}
