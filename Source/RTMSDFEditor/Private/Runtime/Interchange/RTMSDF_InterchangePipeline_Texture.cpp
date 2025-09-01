// Copyright (c) Richard Meredith AB. All Rights Reserved

#include "Runtime/Interchange/RTMSDF_InterchangePipeline_Texture.h"
#include "Importer/Common/RTMSDFTextureSettingsCache.h"
#include "Generation/Bitmap/RTMSDF_BitmapGenerationSettings.h"
#include "Importer/Bitmap/RTMSDF_TexturePostProcess.h"

void URTMSDF_InterchangePipeline_Texture::ExecutePostFactoryPipeline(const UInterchangeBaseNodeContainer* baseNodeContainer, const FString& nodeKey, UObject* createdAsset, bool isReimport)
{
	using namespace RTM::SDF;

	Super::ExecutePostFactoryPipeline(baseNodeContainer, nodeKey, createdAsset, isReimport);

	if(auto* texture = Cast<UTexture2D>(createdAsset))
	{
		FRTMSDFTextureSettingsCache textureSettings(texture);
		const bool isInteresting = TexturePostProcess::IsImportableSDFTexture(texture, FName(texture->GetName()), isReimport);

		if(isInteresting)
		{
			FRTMSDF_BitmapGenerationSettings generationSettings;
			TexturePostProcess::GetExistingGenerationSettings(texture, generationSettings);
			TexturePostProcess::PostProcessImportedTexture(texture, textureSettings, generationSettings, isReimport);
		}
	}
}
