// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once
#include "Importer/Common/RTMSDF_CommonImportSettings.h"

#include "RTMSDF_BitmapImportSettings.generated.h"

UENUM()
enum class ERTMSDF_RGBAMode : uint8
{
	/* Treat each channel separately - either as an SDF or discard */
	SeparateChannels,
	
	/* Preserve the RGB channel and use alpha only to create a distance field. Output will be the same size as input */ 
	PreserveRGB,
};

UENUM(meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor="true"))
enum class ERTMSDF_Channels : uint8
{
	None = 0 UMETA(Hidden),
	Red = 1 << 0,
	Green = 1 << 1,
	Blue = 1 << 2,
	Alpha = 1 << 3,
	RGB = Red | Green | Blue,
	All = Red | Green | Blue | Alpha,
};

USTRUCT(meta=(DisplayName="Bitmap to SDF Import Settings [RTMSDF]"))
struct FRTMSDF_BitmapImportSettings : public FRTMSDF_CommonImportSettings
{
	GENERATED_BODY()

	UPROPERTY()
	int NumChannels = 4;

	/* How to treat the various channels of an RGBA texture */
	UPROPERTY(EditAnywhere, Category="Import", meta=(EditCondition="NumChannels > 1", EditConditionHides, DisplayName="RGBA Mode"))
	ERTMSDF_RGBAMode RGBAMode = ERTMSDF_RGBAMode::SeparateChannels;

	/* Output size of generated SDF texture - for non-square textures this will be the shortest edge */
	UPROPERTY(EditAnywhere, Category="Import", meta=(EditCondition="NumChannels > 1 && RGBAMode == ERTMSDF_RGBAMode::SeparateChannels", EditConditionHides, DisplayAfter="SDFChannels"))
	int TextureSize = 64;
	
	ERTMSDF_Channels GetSDFChannels() { return static_cast<ERTMSDF_Channels>(SDFChannels); }
	bool UsesAnyChannel(ERTMSDF_Channels channelMask) { return 0 != (static_cast<uint8>(channelMask) & SDFChannels); }
	bool UsesChannels(ERTMSDF_Channels channelMask) { return static_cast<uint8>(channelMask) == (static_cast<uint8>(channelMask) & SDFChannels); }

protected:
	/* Which channels should be used to generate SDFs - unused channels will be discarded */
	UPROPERTY(EditAnywhere, Category="Import", meta=(Bitmask, BitmaskEnum=ERTMSDF_Channels, EditCondition="NumChannels > 1 && RGBAMode == ERTMSDF_RGBAMode::SeparateChannels", EditConditionHides, DisplayName="SDF Channels"))
	uint8 SDFChannels = static_cast<uint8>(ERTMSDF_Channels::All);
};
