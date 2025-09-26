// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Generation/Bitmap/RTMSDF_BitmapGenerationSettings.h"

class UTexture2D;
enum ETextureSourceFormat : int;
enum class ERTMSDF_Channels : uint8;
struct FRTMSDFTextureSettingsCache;
struct FRTMSDF_BitmapGenerationSettings;

namespace RTM::SDF::TexturePostProcess
{
	bool GetTextureFormat(ETextureSourceFormat format, TArray<ERTMSDF_Channels, TFixedAllocator<4>>& channelPositions);
	bool GetExistingGenerationSettings(const UTexture2D* existingTexture, FRTMSDF_BitmapGenerationSettings& outGenerationSettings);
	bool IsImportableSDFTexture(UTexture2D* existingTexture, FName assetName, bool isReimport);
	void PostProcessImportedTexture(UTexture2D* texture, FRTMSDFTextureSettingsCache& textureSettings, FRTMSDF_BitmapGenerationSettings& importerSettings, bool isReimport);
}
