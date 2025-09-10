// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once
#include "Generation/Common/RTMSDF_CommonGenerationSettings.h"
#include "Generation/Common/RTMSDF_SDFFormat.h"
#include "RTMSDF_BitmapGenerationSettings.generated.h"

UENUM(DisplayName = "Bitmap Per Channel Behavior [RTMSDF]")
enum class ERTMSDF_BitmapChannelBehavior : uint8
{
	// Treat this channel as an SDF
	SDF,

	// Preserve the original source data
	SourceData,

	// Ignores the data in this channel
	Discard,
};

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
	RGB = Red | Green | Blue UMETA(Hidden),
	All = Red | Green | Blue | Alpha UMETA(Hidden),
};

USTRUCT(meta=(DisplayName="Bitmap to SDF Import Settings [RTMSDF]"))
struct RTMSDF_API FRTMSDF_BitmapGenerationSettings : public FRTMSDF_CommonGenerationSettings
{
	GENERATED_BODY()

	inline static int CurrentVersionNumber = 1;

	UPROPERTY()
	int NumSourceChannels = 4;

	UPROPERTY()
	int VersionNumber = 0;

	UPROPERTY(EditAnywhere, Category="ImportNew", meta=(EditCondition="NumSourceChannels > 1", HideEditConditionToggle, ValidEnumValues="SingleChannel, SeparateChannels"))
	ERTMSDF_SDFFormat Format = ERTMSDF_SDFFormat::SingleChannel;

	// How to handle the red channel from the source data
	UPROPERTY(EditAnywhere, Category="ImportNew", meta=(EditCondition="NumSourceChannels > 1 && Format == ERTMSDF_SDFFormat::SeparateChannels", EditConditionHides))
	ERTMSDF_BitmapChannelBehavior RedChannel = ERTMSDF_BitmapChannelBehavior::SDF;

	// How to handle the green channel from the source data
	UPROPERTY(EditAnywhere, Category="ImportNew", meta=(EditCondition="NumSourceChannels > 1 && Format == ERTMSDF_SDFFormat::SeparateChannels", EditConditionHides))
	ERTMSDF_BitmapChannelBehavior GreenChannel = ERTMSDF_BitmapChannelBehavior::SDF;

	// How to handle the blue channel from the source data
	UPROPERTY(EditAnywhere, Category="ImportNew", meta=(EditCondition="NumSourceChannels > 1 && Format == ERTMSDF_SDFFormat::SeparateChannels", EditConditionHides))
	ERTMSDF_BitmapChannelBehavior BlueChannel = ERTMSDF_BitmapChannelBehavior::SDF;

	// How to handle the alpha channel from the source data
	UPROPERTY(EditAnywhere, Category="ImportNew", meta=(EditCondition="NumSourceChannels > 3 && Format == ERTMSDF_SDFFormat::SeparateChannels", EditConditionHides))
	ERTMSDF_BitmapChannelBehavior AlphaChannel = ERTMSDF_BitmapChannelBehavior::SDF;

	/* Which channel from the source data should be used to generate the SDF
	 * Unused channels will be discarded as this is a single-channel SDF */
	UPROPERTY(EditAnywhere, Category="Import", meta=(EditCondition="NumSourceChannels > 1 && Format == ERTMSDF_SDFFormat::SingleChannel", EditConditionHides, DisplayName="Source Channel"))
	ERTMSDF_Channels SDFChannel = ERTMSDF_Channels::Red;

	/* Output size of generated SDF texture - for non-square textures this will be the shortest edge */
	UPROPERTY(EditAnywhere, Category="Import", meta=(EditCondition="true", EditConditionHides, UIMin=8, ClampMin=8))
	int TextureSize = 64;

	virtual int GetTextureSize() const override { return TextureSize; }
	virtual ERTMSDF_SDFFormat GetFormat() const override { return Format; }

	void SetChannelBehavior(ERTMSDF_Channels channel, ERTMSDF_BitmapChannelBehavior behavior);
	ERTMSDF_BitmapChannelBehavior GetChannelBehavior(ERTMSDF_Channels channel) const;
	ERTMSDF_BitmapChannelBehavior GetSeparatedChannelBehavior(ERTMSDF_Channels channel) const;

	ERTMSDF_Channels GetChannelMapping(ERTMSDF_Channels sdfChannel) const;

	bool LooksLikePreserveRGB() const;
	bool CanScaleSDFTexture() const;
	void FixUpVersioning();

private:
	//~ Deprecated properties remain here as private for fixup
	UPROPERTY()
	int NumSDFChannels = 4;

	/* How to treat the various channels of an RGBA texture */
	UPROPERTY()
	ERTMSDF_RGBAMode RGBAMode = ERTMSDF_RGBAMode::SeparateChannels;

	/* Which channels should be used to generate SDFs - unused channels will be discarded */
	UPROPERTY()
	uint8 SDFChannels = static_cast<uint8>(ERTMSDF_Channels::All);
};
