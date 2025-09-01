// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

#if ENGINE_MAJOR_VERSION >=5 && ENGINE_MINOR_VERSION >=1
#define HAS_DITHER_MIPMAP_ALPHA 0
#else
#define HAS_DITHER_MIPMAP_ALPHA 1
#endif

struct FRTMSDFTextureSettingsCache
{
	// TODO - review all the settings in a texture and set this up properly

	bool SRGB;
	bool bFlipGreenChannel;
	TEnumAsByte<TextureAddress> AddressX;
	TEnumAsByte<TextureAddress> AddressY;
	ETexturePowerOfTwoSetting::Type PowerOfTwoMode;
	FColor PaddingColor;
	TextureFilter Filter;
	TextureGroup LODGroup;
	TextureCompressionSettings CompressionSettings;
	int32 LODBias;
	int32 NumCinematicMipLevels;
	bool NeverStream;
	bool bPreserveBorder;
	bool CompressionNone;
	bool DeferCompression;
	bool CompressionNoAlpha;
#if	HAS_DITHER_MIPMAP_ALPHA
	bool bDitherMipMapAlpha;
#endif
	bool VirtualTextureStreaming;
	FVector4 AlphaCoverageThresholds;

	FRTMSDFTextureSettingsCache(const UTexture2D* texture)
	{
		auto* defaultTexture = GetDefault<UTexture2D>();

		// Force these Settings
		SRGB = false;
		bFlipGreenChannel = false;

		// restore if there is a new value, or set to default value
#define CACHE(field, defaultValue) field = texture ? texture->field : defaultValue
		CACHE(AddressX, TEnumAsByte<TextureAddress>(TA_Clamp));
		CACHE(AddressY, TEnumAsByte<TextureAddress>(TA_Clamp));
#undef  CACHE

#define CACHE(field) field = texture ? texture->field : defaultTexture->field;
		CACHE(PowerOfTwoMode);
		CACHE(PaddingColor);
		CACHE(Filter);
		CACHE(LODGroup);
		CACHE(CompressionSettings);
		CACHE(LODBias);
		CACHE(NumCinematicMipLevels);
		CACHE(NeverStream);
		CACHE(bPreserveBorder);
		CACHE(CompressionNone);
		CACHE(DeferCompression);
		CACHE(CompressionNoAlpha);
#if HAS_DITHER_MIPMAP_ALPHA
		CACHE(bDitherMipMapAlpha);
#endif
		CACHE(VirtualTextureStreaming);
		CACHE(AlphaCoverageThresholds);
#undef  CACHE
	}

	void Restore(UTexture2D* texture) const
	{
#define RESTORE(field) texture->field = field
		RESTORE(SRGB);
		RESTORE(bFlipGreenChannel);
		RESTORE(AddressX);
		RESTORE(AddressY);
		RESTORE(PowerOfTwoMode);
		RESTORE(PaddingColor);
		RESTORE(Filter);
		RESTORE(LODGroup);
		RESTORE(CompressionSettings);
		RESTORE(LODBias);
		RESTORE(NumCinematicMipLevels);
		RESTORE(NeverStream);
		RESTORE(bPreserveBorder);
		RESTORE(CompressionNone);
		RESTORE(DeferCompression);
		RESTORE(CompressionNoAlpha);
#if HAS_DITHER_MIPMAP_ALPHA
		RESTORE(bDitherMipMapAlpha);
#endif
		RESTORE(VirtualTextureStreaming);
		RESTORE(AlphaCoverageThresholds);
#undef  RESTORE
	}
};

#undef HAS_DITHER_MIPMAP_ALPHA