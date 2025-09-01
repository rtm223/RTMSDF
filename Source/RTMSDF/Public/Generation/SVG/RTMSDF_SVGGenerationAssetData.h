// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Generation/Common/RTMSDF_GenerationAssetData_Base.h"
#include "Generation/SVG/RTMSDF_SVGGenerationSettings.h"
#include "RTMSDF_SVGGenerationAssetData.generated.h"

UCLASS(meta=(DisplayName="SVG to MSDF Generation Asset Data [RTMSDF]"))
class RTMSDF_API URTMSDF_SVGGenerationAssetData : public URTMSDF_GenerationAssetData_Base
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Generation", meta=(FullyExpand=true))
	FRTMSDF_SVGGenerationSettings GenerationSettings;

protected:
	virtual const FRTMSDF_CommonGenerationSettings& GetGenerationSettings() const override { return GenerationSettings; }

	virtual void PostLoad() override;
};
