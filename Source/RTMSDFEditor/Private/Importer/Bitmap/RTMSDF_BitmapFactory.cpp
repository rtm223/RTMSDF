// Copyright (c) Richard Meredith AB. All Rights Reserved

#include "RTMSDF_BitmapFactory.h"
#include "Generation/Bitmap/RTMSDF_BitmapGenerationAssetData.h"
#include "Generation/Bitmap/RTMSDF_BitmapGenerationSettings.h"
#include "Importer/Bitmap/RTMSDF_TexturePostProcess.h"
#include "Importer/Common/RTMSDFTextureSettingsCache.h"
#include "Async/ParallelFor.h"
#include "Settings/RTMSDF_ProjectSettings.h"
#include "Curves/CurveLinearColorAtlas.h"
#include "EditorFramework/AssetImportData.h"
#include "Module/RTMSDFEditor.h"

URTMSDF_BitmapFactory::URTMSDF_BitmapFactory()
{
	// Import priority is super high - we want to jump in and test for SDF filenames or user assets
	// before the engine TextureImporter / Reimporter can get hold of the asset

	// TODO - Probably want the reimport priority to be lower than interchange, as interchange is still respecting our high prio
	// Interchange reimporter is going at
	//		return UFactory::GetDefaultImportPriority() + 10;
	// So we can likely go at +9, so will preempt the legacy texture importer, but get steamrolled by interchange

	ImportPriority = GetDefaultImportPriority() + 9;
}

bool URTMSDF_BitmapFactory::FactoryCanImport(const FString& filename)
{
	if(const auto* settings = GetDefault<URTMSDF_ProjectSettings>())
	{
		const auto& suffix = settings->BitmapFilenameSuffix;
		if(suffix.Len() == 0 || FPaths::GetBaseFilename(filename).EndsWith(suffix))
			return true;
	}
	return false;
}

UObject* URTMSDF_BitmapFactory::FactoryCreateBinary(UClass* inClass, UObject* inParent, FName inName, EObjectFlags flags, UObject* context, const TCHAR* type, const uint8*& buffer, const uint8* bufferEnd, FFeedbackContext* warn)
{
	using namespace RTM::SDF;

	auto* existingTexture = FindObject<UTexture2D>(inParent, *inName.ToString());
	const bool isReimport = nullptr != existingTexture;
	const bool isInteresting = TexturePostProcess::IsImportableSDFTexture(existingTexture, inName, isReimport);

	// Cache out existing settings as the legacy importer has a habit of trashing our data
	FRTMSDFTextureSettingsCache textureSettings(existingTexture);
	FRTMSDF_BitmapGenerationSettings generationSettings;
	TexturePostProcess::GetExistingGenerationSettings(existingTexture, generationSettings);

	// let the texture factory do its thing
	UObject* obj = Super::FactoryCreateBinary(inClass, inParent, inName, flags, context, type, buffer, bufferEnd, warn);

	auto* texture = Cast<UTexture2D>(obj);
	if(!isInteresting || !texture)
		return texture;

	// The base texture importer will have called pre and post import internally, no way to get around that, so we broadcast this again as though it's a second import
	GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPreImport(this, inClass, inParent, inName, type);

	TexturePostProcess::PostProcessImportedTexture(texture, textureSettings, generationSettings, isReimport);

	texture->AssetImportData->Update(CurrentFilename, FileHash.IsValid() ? &FileHash : nullptr);

	texture->PostEditChange();
	GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, texture);

	return obj;
}

bool URTMSDF_BitmapFactory::IsAutomatedImport() const
{
	return Super::IsAutomatedImport() || IsAutomatedReimport();
}

int32 URTMSDF_BitmapFactory::GetPriority() const
{
	return ImportPriority;
}

bool URTMSDF_BitmapFactory::CanReimport(UObject* obj, TArray<FString>& outFilenames)
{
	UTexture* tex = Cast<UTexture2D>(obj);
	if(tex && !tex->IsA<UCurveLinearColorAtlas>() && tex->GetAssetUserData<URTMSDF_BitmapGenerationAssetData>())
	{
		tex->AssetImportData->ExtractFilenames(outFilenames);
		return true;
	}
	return false;
}

void URTMSDF_BitmapFactory::SetReimportPaths(UObject* obj, const TArray<FString>& newReimportPaths)
{
	UTexture* tex = Cast<UTexture2D>(obj);
	if(tex && ensure(newReimportPaths.Num() == 1))
		tex->AssetImportData->UpdateFilenameOnly(newReimportPaths[0]);
}

EReimportResult::Type URTMSDF_BitmapFactory::Reimport(UObject* obj)
{
	if(auto* texture = Cast<UTexture2D>(obj))
	{
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

		bool outCancelled = false;
		UTextureFactory::SuppressImportOverwriteDialog();
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

	return EReimportResult::Type::Failed;
}
