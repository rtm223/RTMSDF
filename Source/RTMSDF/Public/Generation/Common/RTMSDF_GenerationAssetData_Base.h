// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetUserData.h"
#include "Generation/Common/RTMSDF_CommonGenerationSettings.h"
#include "RTMSDF_GenerationAssetData_Base.generated.h"

UCLASS(Abstract, meta=(DisplayName="Bitmap to SDF Generation Asset Data [RTMSDF]"))
class RTMSDF_API URTMSDF_GenerationAssetData_Base : public UAssetUserData
{
	GENERATED_BODY()

public:
	virtual const FRTMSDF_CommonGenerationSettings& GetGenerationSettings() const PURE_VIRTUAL(URTMSDF_GenerationAssetData_Base::GetGenerationSettings, return DefaultSettings;)

	// Automatically calculated property. Useful for calculations in materials. Can be accessed using the Blueprint Function Library
	UPROPERTY(VisibleAnywhere, AdvancedDisplay, Category="Cached", meta=(DisplayName = "UV Range"))
	float UVRange = -1.0f;

	// Automatically calculated property. Useful for calculations in materials. Can be accessed using the Blueprint Function Library
	UPROPERTY(VisibleAnywhere, AdvancedDisplay, Category="Cached")
	FIntPoint SourceDimensions = {-1, -1};

private:
	inline static FRTMSDF_CommonGenerationSettings DefaultSettings;
};
