// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Generation/Common/RTMSDF_GenerationAssetData_Base.h"
#include "Generation/Bitmap/RTMSDF_BitmapGenerationSettings.h"
#include "RTMSDF_BitmapGenerationAssetData.generated.h"

UCLASS(meta=(DisplayName="Bitmap to SDF Generation Asset Data [RTMSDF]"))
class RTMSDF_API URTMSDF_BitmapGenerationAssetData : public URTMSDF_GenerationAssetData_Base
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Generation", meta=(FullyExpand=true))
	FRTMSDF_BitmapGenerationSettings GenerationSettings;

protected:
	virtual const FRTMSDF_CommonGenerationSettings& GetGenerationSettings() const override { return GenerationSettings; }

	virtual void PostLoad() override;
};
