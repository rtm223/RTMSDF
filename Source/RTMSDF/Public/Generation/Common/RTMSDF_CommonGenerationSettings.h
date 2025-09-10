// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

#include "RTMSDF_CommonGenerationSettings.generated.h"

enum class ERTMSDF_SDFFormat : uint8;
UENUM(DisplayName = "MSDF Distance Mode [RTMSDF]")
enum class ERTMSDFDistanceMode : uint8
{
	/* Normalized against source file (shortest edge)
	 * Recommended as this will give consistent results in materials if you change the source or target resolution of the data */
	Normalized,
	// Absolute range in source file
	Absolute,
	// Absolute range in output SDF
	Pixels,
};

USTRUCT(meta=(DisplayName="Common SDF Import Settings [RTMSDF]"))
struct FRTMSDF_CommonGenerationSettings
{
	GENERATED_BODY()

	/* How the distance field size is determined */
	UPROPERTY(EditAnywhere, Category="Import")
	ERTMSDFDistanceMode DistanceMode = ERTMSDFDistanceMode::Normalized;

	/* Distance field size normalized against shortest edge of texture, used if distance mode = Normalized
	 * Recommended as this will give consistent results in materials if you change the source or target resolution of the data */
	UPROPERTY(EditAnywhere, Category="Import", meta=(EditCondition="DistanceMode == ERTMSDFDistanceMode::Normalized || bIsInProjectSettings", EditConditionHides, UIMin=0, ClampMin=0, UIMax=0.45, ClampMax=0.45))
	float NormalizedDistance = 0.125f;

	/* Distance field size as an absolute size in the source file, used if distance mode = Absolute */
	UPROPERTY(EditAnywhere, Category="Import", meta=(EditCondition="DistanceMode == ERTMSDFDistanceMode::Absolute || bIsInProjectSettings", EditConditionHides, UIMin=1, ClampMin=1))
	float AbsoluteDistance = 64.0f;

	/* Distance field size as pixels in the output SDF texture, used if distance mode =  Pixels */
	UPROPERTY(EditAnywhere, Category="Import", meta=(EditCondition="DistanceMode == ERTMSDFDistanceMode::Pixels || bIsInProjectSettings", EditConditionHides, UIMin=1, ClampMin=1))
	float PixelDistance = 16.0f;

	/* Invert distance? (pixels inside the shape will be +ve, outside -ve) */
	UPROPERTY(EditAnywhere, Category="Import")
	bool bInvertDistance = false;

	// Adjusts the size of the output distance field to accomodate the distance field. If false, source assets will need to be authored to contain suitable margins
	UPROPERTY(EditAnywhere, Category="Import", meta=(EditCondition="True", EditConditionHides, DisplayAfter="bInvertDistance"))
	bool bScaleToFitDistance = true;

	UPROPERTY()
	bool bIsInProjectSettings = false;

	virtual ~FRTMSDF_CommonGenerationSettings() = default;

	virtual int GetTextureSize() const { return 0; }
	virtual ERTMSDF_SDFFormat GetFormat() const { return static_cast<ERTMSDF_SDFFormat>(0); }

	double GetAbsoluteRange(const FVector2D& inputDimensions) const
	{
		const double textureSize = GetTextureSize();
		const double minEdge = FMath::Min(inputDimensions.X, inputDimensions.Y);
		const double scale = textureSize / minEdge;

		if(DistanceMode == ERTMSDFDistanceMode::Normalized)
			return NormalizedDistance * minEdge;

		if(DistanceMode == ERTMSDFDistanceMode::Pixels)
			return PixelDistance / scale;

		return AbsoluteDistance;
	}

	double GetNormalizedRange(const FVector2D& inputDimensions) const
	{
		// NOTE - this will break down for preserve RGB bitmaps as they do not scale like others.
		// Right now that is fixed in the import stage by trampling the texture size to be equal to the input dimensions

		// Note - this probably wants to also be cached out at import, if we want to get the normalized / UV range at runtime for blueprints

		const double textureSize = GetTextureSize();
		const double minEdge = FMath::Min(inputDimensions.X, inputDimensions.Y);
		const double scale = textureSize / minEdge;

		if(DistanceMode == ERTMSDFDistanceMode::Absolute)
			return AbsoluteDistance / minEdge;

		if(DistanceMode == ERTMSDFDistanceMode::Pixels)
			return PixelDistance / textureSize;

		return NormalizedDistance;
	}

	//~ Note: no pixel range conversion is added as it's never used anywhere (MSDF wants Absolute and the Bitmap version wants normalized)
};
