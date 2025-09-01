// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

#include "Generation/Common/RTMSDF_CommonGenerationSettings.h"
#include "Generation/Common/RTMSDF_SDFFormat.h"
#include "Generation/SVG/RTM_MSDFEnums.h"
#include "RTMSDF_SVGGenerationSettings.generated.h"

USTRUCT(meta=(DisplayName="SVG to SDF Import Settings [RTMSDF]"))
struct RTMSDF_API FRTMSDF_SVGGenerationSettings : public FRTMSDF_CommonGenerationSettings
{
	GENERATED_BODY()

	inline static int CurrentVersionNumber = 1;

	UPROPERTY()
	int VersionNumber = 0;

	UPROPERTY(EditAnywhere, Category="Import", meta=(ValidEnumValues="SingleChannel, SingleChannelPseudo, Multichannel, MultichannelPlusAlpha"))
	ERTMSDF_SDFFormat Format = ERTMSDF_SDFFormat::MultichannelPlusAlpha;

	/* Output size of generated SDF texture - for non-square textures this will be the shortest edge - recommend power of 2 sizes only */
	UPROPERTY(EditAnywhere, Category="Import")
	int TextureSize = 64;

	UPROPERTY(EditAnywhere, Category="Import", meta=(EditCondition="Format == ERTMSDFFormat::Multichannel || Format == ERTMSDFFormat::MultichannelPlusAlpha", DisplayAfter="InvertDistance"))
	ERTMSDF_MSDFColoringMode EdgeColoringMode = ERTMSDF_MSDFColoringMode::Distance;

	UPROPERTY(EditAnywhere, Category="Import", meta=(EditCondition="Format == ERTMSDFFormat::Multichannel || Format == ERTMSDFFormat::MultichannelPlusAlpha", DisplayAfter="InvertDistance"))
	int EdgeColoringSeed = 0;

	/* Maximum angle to treat a corner as a corner for the sake of edge coloring / preserving sharpness*/
	UPROPERTY(EditAnywhere, Category="Import", meta=(EditCondition="Format == ERTMSDFFormat::Multichannel || Format == ERTMSDFFormat::MultichannelPlusAlpha", DisplayAfter="InvertDistance", UIMin=1, ClampMin=1, UIMax=179, ClampMax=179))
	float MaxCornerAngle = 175.0f;

	UPROPERTY(EditAnywhere, Category="Import", meta=(EditCondition="Format == ERTMSDFFormat::Multichannel || Format == ERTMSDFFormat::MultichannelPlusAlpha", DisplayAfter="InvertDistance"))
	ERTMSDF_MSDFErrorCorrectionMode ErrorCorrectionMode = ERTMSDF_MSDFErrorCorrectionMode::EdgePriorityFull;

	UPROPERTY(EditAnywhere, Category="Import", meta=(EditCondition="Format == ERTMSDFFormat::Multichannel || Format == ERTMSDFFormat::MultichannelPlusAlpha", DisplayAfter="InvertDistance", UIMin=1, ClampMin=1, UIMax=2, ClampMax=2))
	double MinErrorDeviation = 1.11111111111111111;

	UPROPERTY(EditAnywhere, Category="Import", meta=(EditCondition="Format == ERTMSDFFormat::Multichannel || Format == ERTMSDFFormat::MultichannelPlusAlpha", DisplayAfter="InvertDistance", UIMin=1, ClampMin=1, UIMax=2, ClampMax=2))
	double MinErrorImprovement = 1.11111111111111111;

	virtual int GetTextureSize() const override { return TextureSize; }
	virtual ERTMSDF_SDFFormat GetFormat() const override { return Format; }
	void FixUpVersioning();
};
