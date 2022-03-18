// Copyright (c) Richard Meredith AB. All Rights Reserved

#include "RTMSDF_SVGFactory.h"
#include "AssetImportTask.h"
#include "RTMSDF_SVGGenerationHelpers.h"
#include "Importer/RTMSDFTextureSettingsCache.h"
#include "RTMSDF_SVGImportAssetData.h"
#include "RTMSDF_SVGImportSettings.h"
#include "ChlumskyMSDFGen/Public/Core/msdfgen.h"
#include "ChlumskyMSDFGen/Public/Ext/import-svg.h"
#include "Config/RTMSDFConfig.h"
#include "Curves/CurveLinearColorAtlas.h"
#include "EditorFramework/AssetImportData.h"
#include "Engine/Texture2DArray.h"
#include "Module/RTMSDFEditor.h"

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
	if(const auto* settings = GetDefault<URTMSDFConfig>())
	{
		const auto& suffix = settings->SVGFilenameSuffix;
		if(suffix.Len() == 0 || filename.EndsWith(suffix))
			return true;
	}
	return false;
}

UObject* URTMSDF_SVGFactory::FactoryCreateBinary(UClass* inClass, UObject* inParent, FName inName, EObjectFlags flags, UObject* context, const TCHAR* type, const uint8*& buffer, const uint8* bufferEnd, FFeedbackContext* warn)
{
	using namespace msdfgen;
	using namespace RTMSDFGenerationHelpers;

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
	FRTMSDF_SVGImportSettings importerSettings;
	if(const auto* previousSettings = existingTexture ? existingTexture->GetAssetUserData<URTMSDF_SVGImportAssetData>() : nullptr)
	{
		importerSettings = previousSettings->ImportSettings;
	}
	else if(const auto* defaultConfig = GetDefault<URTMSDFConfig>())
	{
		importerSettings = defaultConfig->DefaultSVGImportSettings;
		textureSettings.LODGroup = defaultConfig->SVGTextureGroup;
	}

	Shape shape;
	Vector2 svgDims;
	if(!CreateShape(buffer, bufferEnd, shape, svgDims))
	{
		UE_LOG(RTMSDFEditor, Error, TEXT("Import for %s failed - unable to create Shape"), *inName.ToString());
		GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, nullptr);
		return nullptr;
	}

	shape.normalize();
	// TODO VAlidate shape: shape.validate()
	// 	shape.validate();
	

	UTexture2D* texture = nullptr;
	if(auto newObject = CreateOrOverwriteAsset(inClass, inParent, inName, flags))
		texture = CastChecked<UTexture2D>(newObject);

	// NOTE: existingTexture will now point to the new texture - don't try to use it's values after this

	if(!texture)
	{
		if(existingTexture)
			existingTexture->UpdateResource();

		UE_LOG(RTMSDFEditor, Error, TEXT("Import for %s failed - unable to create Texture"), *inName.ToString());
		GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, nullptr);
		return nullptr;
	}

	Vector2 scale = (double)(importerSettings.TextureSize) / min(svgDims.x, svgDims.y);
	Vector2 msdfDims = svgDims * scale;
	Projection projection(scale, 0.0f);

#ifdef MSDFGEN_USE_SKIA
	// TODO: SKIA : resolve shape goes here
	static_assert(false, "Skia not implemented, but building with 'MSDFGEN_USE_SKIA")
#else
	shape.orientContours();
#endif
	
	double range = importerSettings.AbsoluteDistance;
	if(importerSettings.DistanceMode == ERTMSDFDistanceMode::Normalized)
		range = importerSettings.NormalizedDistance * min(svgDims.x, svgDims.y);
	else if(importerSettings.DistanceMode == ERTMSDFDistanceMode::Pixels)
		range = importerSettings.PixelDistance / min(scale.x, scale.y);

	if(importerSettings.Format == ERTMSDFFormat::Multichannel || importerSettings.Format == ERTMSDFFormat::MultichannelPlusAlpha)
		DoEdgeColoring(shape, importerSettings.EdgeColoringMode, FMath::DegreesToRadians(importerSettings.MaxCornerAngle), importerSettings.EdgeColoringSeed);

	MSDFGeneratorConfig generatorConfig;
	generatorConfig.overlapSupport = true;
	ApplyErrorCorrectionModeTo(generatorConfig.errorCorrection, importerSettings.ErrorCorrectionMode);
	Generate(importerSettings.Format, generatorConfig, msdfDims, shape, projection, range, importerSettings.InvertDistance, texture);

	if(!existingTexture)
	{
		UE_LOG(RTMSDFEditor, Log, TEXT("Fresh import of %s - applying default SDF settings"), *texture->GetPathName())
	}
	
	UpdateNewTextureSettings(texture, textureSettings, importerSettings.Format);

	texture->AssetImportData->Update(CurrentFilename, FileHash.IsValid() ? &FileHash : nullptr);

	if(!texture->GetAssetUserData<URTMSDF_SVGImportAssetData>())
	{
		auto importData = NewObject<URTMSDF_SVGImportAssetData>(texture, NAME_None, flags);
		importData->ImportSettings = importerSettings;
		texture->AddAssetUserData(importData);
	}

	texture->bHasBeenPaintedInEditor = false;

	RefReplacer.Replace(texture);

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

	return texture;
}

int32 URTMSDF_SVGFactory::GetPriority() const
{
	return INT32_MAX;
}

bool URTMSDF_SVGFactory::CanReimport(UObject* obj, TArray<FString>& outFilenames)
{
	auto* tex = Cast<UTexture2D>(obj);
	if(tex && !tex->IsA<UCurveLinearColorAtlas>() && tex->GetAssetUserData<URTMSDF_SVGImportAssetData>())
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
