// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

#include "ChlumskyMSDFGen/Public/Core/generator-config.h"
#include "Importer/Common/RTMSDF_CommonImportSettings.h"
#include "RTMSDF_SVGImportSettings.generated.h"

UENUM(DisplayName = "MSDF Format [RTMSDF]")
enum class ERTMSDFFormat : uint8
{
	// Traditional 'true' SDF as Alpha (BC4)
	SingleChannel,
	// PseudoSDF 
	SingleChannelPseudo,
	// MSDF in Uncompressed RGBA (alpha unused)
	Multichannel,
	// MSDF in Uncompressed RGBA with true SDF in A 
	MultichannelPlusAlpha,
};

UENUM(DisplayName = "MSDF Coloring Mode [RTMSDF]")
enum class ERTMSDFColoringMode : uint8
{
	// None just produces a PSDF in BGRA, so effectively useless
	None UMETA(Hidden),
	
	// Simplest method, uses angle threshold (3 rads) to test for corners.
	Simple,
	
	// Specialized for fonts with ink traps as features
	InkTrap,
	
	// Theoretically best, but slowest
	Distance,
};

UENUM(DisplayName = "MSDF Error Correction Mode [RTMSDF]")
enum class ERTMSDFErrorCorrectionMode : uint8
{
	// Error Correction Disabled
	None,

	// Only correct artifacts at edges. No distance checks
	EdgeOnlyFast UMETA(DisplayName="Edge Only - Fast"),

	// Only correct artifacts at edges, Distance checks at edges only
	EdgeOnlyBalanced UMETA(DisplayName="Edge Only - Balanced"),

	// Only correct artifacts at edges. Full distance checks
	EdgeOnlyFull UMETA(DisplayName="Edge Only - Full"),

	// Correct all artifacts, but prioritise preserving edges and corners. No distance checks
	EdgePriorityFast UMETA(DisplayName="Edge Priority - Fast"),

	// Correct all artifacts, but prioritise preserving edges and corners. Full distance checks
	EdgePriorityFull UMETA(DisplayName="Edge Priority - Full"),

	// Correct all artifacts. No distance checks
	IndiscriminateFast UMETA(DisplayName="Indiscriminate - Fast"),

	// Correct all artifacts. Full distance checks
	IndiscriminateFull UMETA(DisplayName="Indiscriminate - Full"),
};

USTRUCT(BlueprintType, meta=(DisplayName="SVG to SDF Import Settings [RTMSDF]"))
struct FRTMSDF_SVGImportSettings : public FRTMSDF_CommonImportSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Import")
	ERTMSDFFormat Format = ERTMSDFFormat::Multichannel;

	/* Output size of generated SDF texture - for non-square textures this will be the shortest edge - recommend power of 2 sizes only */
	UPROPERTY(EditAnywhere, Category="Import")
	int TextureSize = 32;

	UPROPERTY(EditAnywhere, Category="Import", meta=(EditCondition="Format == ERTMSDFFormat::Multichannel || Format == ERTMSDFFormat::MultichannelPlusAlpha", DisplayAfter="InvertDistance"))
	ERTMSDFColoringMode EdgeColoringMode = ERTMSDFColoringMode::Distance;

	UPROPERTY(EditAnywhere, Category="Import", meta=(EditCondition="Format == ERTMSDFFormat::Multichannel || Format == ERTMSDFFormat::MultichannelPlusAlpha", DisplayAfter="InvertDistance"))
	int EdgeColoringSeed = 0;

	/* Maximum angle to treat a corner as a corner for the sake of edge coloring / preserving sharpness*/
	UPROPERTY(EditAnywhere, Category="Import", meta=(EditCondition="Format == ERTMSDFFormat::Multichannel || Format == ERTMSDFFormat::MultichannelPlusAlpha", DisplayAfter="InvertDistance", UIMin=1, ClampMin=1, UIMax=179, ClampMax=179))
	float MaxCornerAngle = 175.0f;

	UPROPERTY(EditAnywhere, Category="Import", meta=(EditCondition="Format == ERTMSDFFormat::Multichannel || Format == ERTMSDFFormat::MultichannelPlusAlpha", DisplayAfter="InvertDistance"))
	ERTMSDFErrorCorrectionMode ErrorCorrectionMode = ERTMSDFErrorCorrectionMode::EdgePriorityFull;

	UPROPERTY(EditAnywhere, Category="Import", meta=(EditCondition="Format == ERTMSDFFormat::Multichannel || Format == ERTMSDFFormat::MultichannelPlusAlpha", DisplayAfter="InvertDistance", UIMin=1, ClampMin=1, UIMax=2, ClampMax=2))
	float MinErrorDeviation = msdfgen::ErrorCorrectionConfig::defaultMinDeviationRatio;

	UPROPERTY(EditAnywhere, Category="Import", meta=(EditCondition="Format == ERTMSDFFormat::Multichannel || Format == ERTMSDFFormat::MultichannelPlusAlpha", DisplayAfter="InvertDistance", UIMin=1, ClampMin=1, UIMax=2, ClampMax=2))
	float MinErrorImprovement = msdfgen::ErrorCorrectionConfig::defaultMinDeviationRatio;
};
