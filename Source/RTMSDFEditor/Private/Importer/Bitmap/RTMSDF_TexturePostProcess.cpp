// Copyright (c) Richard Meredith AB. All Rights Reserved

#include "Importer/Bitmap/RTMSDF_TexturePostProcess.h"
#include "EditorFramework/AssetImportData.h"
#include "Generation/Bitmap/RTMSDF_BitmapGenerationAssetData.h"
#include "Generation/Bitmap/RTMSDF_BitmapGenerationSettings.h"
#include "settings/RTMSDF_ProjectSettings.h"
#include "Importer/Common/RTMSDFTextureSettingsCache.h"
#include "Module/RTMSDFEditor.h"
#include "Generation/Bitmap/RTMSDF_BitmapGeneration.h"
#include "Generation/Common/RTMSDF_Buffers.h"

namespace RTM::SDF::TexturePostProcess
{
	bool HACK_ShouldForceSingleChannelOutput(const UTexture2D* texture, bool isReimport)
	{
		// HACK - force new PSDs to be single channel, as unreal doesn't parse the number of channels from the header, and single channel is probably more likely
		// TODO - steal the PSD parser from InterchangePSDTranslator.cpp and find the actual channel width

		if(isReimport)
			return false;
		const auto* importData = texture->AssetImportData.Get();

		if(!importData)
			return false;

		const auto& sourceFiles = importData->SourceData.SourceFiles;

		return sourceFiles.Num() == 1
			&& sourceFiles[0].RelativeFilename.EndsWith(".psd");
	}

	bool GetTextureFormat(ETextureSourceFormat format, TArray<ERTMSDF_Channels, TFixedAllocator<4>>& channelPositions)
	{
		switch(format)
		{
			case TSF_G8:
				channelPositions.Add(ERTMSDF_Channels::Red);
				return true;

			case TSF_BGRA8:
			case TSF_BGRE8:		// unsure what the e is for?
				channelPositions.Append({ERTMSDF_Channels::Blue, ERTMSDF_Channels::Green, ERTMSDF_Channels::Red, ERTMSDF_Channels::Alpha});
				return true;

			case TSF_RGBA16:
			case TSF_RGBA16F:
			case TSF_G16:
				UE_LOG(RTMSDFEditor, Error, TEXT("Unsupported Source format - 16 bit formats not currently supported"));
			default:
				UE_LOG(RTMSDFEditor, Error, TEXT("Unable to import file with texture format (ETextureSourceFormat)  = %d"), static_cast<int>(format));
				return false;
		}
	}

	bool GetExistingGenerationSettings(const UTexture2D* existingTexture, FRTMSDF_BitmapGenerationSettings& outGenerationSettings)
	{
		// TODO - replace this with a proper seach through the AssetUser data, so we don't have to bypass the non-constness of GetAssetUserData;
		auto* nonConstTexture = const_cast<UTexture2D*>(existingTexture);
		if(const auto* previousSettings = existingTexture ? nonConstTexture->GetAssetUserData<URTMSDF_BitmapGenerationAssetData>() : nullptr)
		{
			outGenerationSettings = previousSettings->GenerationSettings;
			return true;
		}

		return false;
	}

	bool IsImportableSDFTexture(UTexture2D* existingTexture, FName assetName, bool isReimport)
	{
		if(const auto* previousSettings = existingTexture ? existingTexture->GetAssetUserData<URTMSDF_BitmapGenerationAssetData>() : nullptr)
			return true;

		// Only create default settings if this is the first import
		if(isReimport)
			return false;

		if(const auto* defaultConfig = GetDefault<URTMSDF_ProjectSettings>())
		{
			const auto& suffix = defaultConfig->BitmapFilenameSuffix;
			if(suffix.Len() > 0 && assetName.ToString().EndsWith(suffix))
				return true;
		}

		return false;
	}

	void PostProcessImportedTexture(UTexture2D* texture, FRTMSDFTextureSettingsCache& textureSettings, FRTMSDF_BitmapGenerationSettings& importerSettings, bool isReimport)
	{
		const uint64 cyclesStart = FPlatformTime::Cycles();

		const bool forceSingleChannelOutput = HACK_ShouldForceSingleChannelOutput(texture, isReimport);
		texture->bHasBeenPaintedInEditor = false;

		// here we will the place where we do our stuff - grab the source data, rebuild it and overwrite, and attach a data to it.

		uint8* mip = texture->Source.LockMip(0, 0, 0);
		const ETextureSourceFormat sourceFormat = texture->Source.GetFormat(0);
		const int numSourceChannels = texture->Source.GetBytesPerPixel(0);		// TODO - this is currently ok because we are only supporting 8 bit source files
		const int sourceWidth = texture->Source.GetSizeX();
		const int sourceHeight = texture->Source.GetSizeY();
		const FVector2D sourceSize(sourceWidth, sourceHeight);

		TArray<ERTMSDF_Channels, TFixedAllocator<4>> sourceChannelColors;
		if(!GetTextureFormat(sourceFormat, sourceChannelColors))
		{
			// unlock and bail;
			texture->Source.UnlockMip(0, 0, 0);
			return;
		}

		ensureAlways(numSourceChannels == sourceChannelColors.Num());

		if(!isReimport)
		{
			if(const auto* projectSettings = GetDefault<URTMSDF_ProjectSettings>())
			{
				const bool isSingleChannelSDF = forceSingleChannelOutput || numSourceChannels == 1;
				importerSettings = isSingleChannelSDF ? projectSettings->DefaultBitmapImportSettings_SingleChannel : projectSettings->DefaultBitmapImportSettings_MultiChannel;
				importerSettings.bIsInProjectSettings = false;
				textureSettings.LODGroup = projectSettings->BitmapTextureGroup;
				textureSettings.AddressX = TA_Clamp;
				textureSettings.AddressY = TA_Clamp;
				textureSettings.SRGB = false;
			}
		}

		importerSettings.NumSourceChannels = numSourceChannels;
		if(importerSettings.NumSourceChannels == 1)
			importerSettings.Format = ERTMSDF_SDFFormat::SingleChannel;

		const int numSDFChannels = (forceSingleChannelOutput || IsSingleChannelFormat(importerSettings.Format)) ? 1 : 4;
		ETextureSourceFormat sdfFormat = numSDFChannels == 1 ? TSF_G8 : TSF_BGRA8;
		TArray<ERTMSDF_Channels, TFixedAllocator<4>> sdfChannelColors;
		GetTextureFormat(sdfFormat, sdfChannelColors);
		ensureAlways(numSDFChannels == sdfChannelColors.Num());

		const bool canScaleSDF = importerSettings.CanScaleSDFTexture();
		if(!canScaleSDF)
		{
			importerSettings.TextureSize = FMath::Min(sourceWidth, sourceHeight);
			importerSettings.bScaleToFitDistance = false;
		}

		const float scale = importerSettings.TextureSize / static_cast<float>(FMath::Min(sourceWidth, sourceHeight));
		const double normalizedDistance = importerSettings.GetNormalizedRange(sourceSize);

		const FSDFBufferDef sourceBufferDef(sourceWidth, sourceHeight, numSourceChannels, sourceFormat);
		const FSDFBufferDef sdfBufferDef(sourceBufferDef.Width * scale, sourceBufferDef.Height * scale, numSDFChannels, sdfFormat);

		{
			// TODO - work out how to map this better than what we are doing. Probably roll into the non-square update?

			const int bufferLen = sdfBufferDef.GetBufferLen();
			uint8* sdfPixels = static_cast<uint8*>(FMemory::Malloc(bufferLen));
			ON_SCOPE_EXIT { FMemory::Free(sdfPixels); };

			FMemory::Memset(sdfPixels, 0, bufferLen);

			const float sdfAreaScale = importerSettings.bScaleToFitDistance ? 1.0f / (1.0f - 2.0f * normalizedDistance) : 1.0f;
			const bool tileX = texture->AddressX == TA_Wrap;
			const bool tileY = texture->AddressY == TA_Wrap;

			for(int i = 0; i < numSDFChannels; ++i)
			{
				const auto sdfChannel = sdfChannelColors[i];

				const ERTMSDF_Channels sourceChannel = importerSettings.GetChannelMapping(sdfChannel);
				ERTMSDF_BitmapChannelBehavior behavior = importerSettings.GetChannelBehavior(sourceChannel);

				const int sourceChannelIdx = sourceChannelColors.Find(sourceChannel);

				if(sourceChannelIdx != INDEX_NONE)
				{
					if(behavior == ERTMSDF_BitmapChannelBehavior::SDF)
					{
						const FSDFBufferMapping bufferMap(sourceChannelIdx, i, normalizedDistance, tileX, tileY, sdfAreaScale, importerSettings.bInvertDistance);
						const bool success = CreateDistanceField(mip, sourceBufferDef, sdfPixels, sdfBufferDef, bufferMap);
						if(!success)
							behavior = ERTMSDF_BitmapChannelBehavior::Discard;
					}
					else if(behavior == ERTMSDF_BitmapChannelBehavior::SourceData)
					{
						if(ensureAlways(scale == 1.0f))
						{
							CopyChannelValues(mip, sourceBufferDef, sourceChannelIdx, sdfPixels, sdfBufferDef, i);
						}
						else
						{
							behavior = ERTMSDF_BitmapChannelBehavior::Discard;
						}
					}
				}

				importerSettings.SetChannelBehavior(sourceChannel, behavior);

				// Other channels default to 0 (memset above). Alpha channel goes to 1.0 if otherwise the texture preview is default unusable
				if(behavior == ERTMSDF_BitmapChannelBehavior::Discard && sdfChannelColors[i] == ERTMSDF_Channels::Alpha)
					SetChannelUniformValue(sdfPixels, sdfBufferDef, i, 255);
			}

			// TODO - pass in the format here
			texture->Source.UnlockMip(0, 0, 0);
			texture->Source.Init(sdfBufferDef.Width, sdfBufferDef.Height, 1, 1, sdfFormat, sdfPixels);
		}

		textureSettings.CompressionSettings = numSDFChannels == 1 ? TC_Grayscale : TC_EditorIcon;

		auto* assetData = texture->GetAssetUserData<URTMSDF_BitmapGenerationAssetData>();
		if(!assetData)
		{
			assetData = NewObject<URTMSDF_BitmapGenerationAssetData>(texture, NAME_None, texture->GetFlags());
			texture->AddAssetUserData(assetData);
		}

		assetData->GenerationSettings = importerSettings;
		assetData->UVRange = normalizedDistance;
		assetData->SourceDimensions = {sourceWidth, sourceHeight};

		if(!texture)
		{
			UE_LOG(RTMSDFEditor, Log, TEXT("Fresh import of %s - applying default SDF settings"), *texture->GetPathName())
		}

		textureSettings.Restore(texture);

		const uint64 cyclesEnd = FPlatformTime::Cycles();

		UE_LOG(RTMSDFEditor, Log, TEXT("Import Complete - %.2f miliseconds"), FPlatformTime::ToMilliseconds(cyclesEnd-cyclesStart));
	}
}
