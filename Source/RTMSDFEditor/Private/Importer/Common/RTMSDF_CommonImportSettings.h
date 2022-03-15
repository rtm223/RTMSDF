// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

#include "RTMSDF_CommonImportSettings.generated.h"

UENUM(DisplayName = "MSDF Distance Mode [RTMSDF]")
enum class ERTMSDFDistanceMode : uint8
{
	// Normalized against source file (shortest edge)
	Normalized,
	// Absolute range in source file
	Absolute,
	// Absolute range in output SDF
	Pixels,
};

USTRUCT(meta=(DisplayName="Common SDF Import Settings [RTMSDF]"))
struct FRTMSDF_CommonImportSettings
{
	GENERATED_BODY()

	/* How the distance field size is determined */
	UPROPERTY(EditAnywhere, Category="Import")
	ERTMSDFDistanceMode DistanceMode = ERTMSDFDistanceMode::Normalized;

	/* Distance field size normalized against shortest edge of texture */
	UPROPERTY(EditAnywhere, Category="Import", meta=(EditCondition="DistanceMode == ERTMSDFDistanceMode::Normalized", EditConditionHides, UIMin=0, ClampMin=0, UIMax=0.5, ClampMax=0.5))
	float NormalizedDistance = 0.25f;

	/* Distance field size as an absolute size in the source file */
	UPROPERTY(EditAnywhere, Category="Import", meta=(EditCondition="DistanceMode == ERTMSDFDistanceMode::Absolute", EditConditionHides, UIMin=1, ClampMin=1))
	float AbsoluteDistance = 128.0f;

	/* Distance field size as pixels in the output SDF texture */
	UPROPERTY(EditAnywhere, Category="Import", meta=(EditCondition="DistanceMode == ERTMSDFDistanceMode::Pixels", EditConditionHides, UIMin=1, ClampMin=1))
	float PixelDistance = 4.0f;

	/* Invert distance? (pixels inside the shape will be +ve, outside -ve) */
	UPROPERTY(EditAnywhere, Category="Import")
	bool InvertDistance = true;
};
