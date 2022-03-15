// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "RTMSDF_BitmapImportSettings.h"
#include "RTMSDF_BitmapImportAssetData.generated.h"

UCLASS(meta=(DisplayName="Bitmap to SDF Import Asset Data [RTMSDF]"))
class URTMSDF_BitmapImportAssetData : public UAssetUserData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Import", meta=(FullyExpand=true))
	FRTMSDF_BitmapImportSettings ImportSettings;
	
	virtual bool IsEditorOnly() const override { return true; }
};
