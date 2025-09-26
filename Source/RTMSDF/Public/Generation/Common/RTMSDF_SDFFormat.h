// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once
#include "CoreMinimal.h"

UENUM(BlueprintType, DisplayName = "SDF Format [RTMSDF]")
enum class ERTMSDF_SDFFormat : uint8
{
	// Traditional 'true' SDF
	SingleChannel,

	// Pseudo SDF - single channel SDF with features similar to MSDF / Multichannel (sharp corners etc.)
	SingleChannelPseudo,

	/* Treat each channel (RGBA) separately - either as an SDF, the original source data or discard */
	SeparateChannels,

	// MSDF in RGB
	Multichannel,

	// MSDF in RGBA with true SDF in the Alpha channel
	MultichannelPlusAlpha,

	MAX UMETA(Hidden),

	Invalid,
};

inline bool IsSingleChannelFormat(ERTMSDF_SDFFormat format)
{
	return format == ERTMSDF_SDFFormat::SingleChannel
		|| format == ERTMSDF_SDFFormat::SingleChannelPseudo;
}