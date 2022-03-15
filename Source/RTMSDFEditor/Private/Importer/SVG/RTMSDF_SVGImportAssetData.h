// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "RTMSDF_SVGImportSettings.h"
#include "RTMSDF_SVGImportAssetData.generated.h"

UCLASS(meta=(DisplayName="SVG to SDF Import Asset Data [RTMSDF]"))
class URTMSDF_SVGImportAssetData : public UAssetUserData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Import", meta=(FullyExpand=true))
	FRTMSDF_SVGImportSettings ImportSettings;

	virtual bool IsEditorOnly() const override { return true; }
};
