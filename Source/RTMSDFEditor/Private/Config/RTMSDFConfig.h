// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Importer/Bitmap/RTMSDF_BitmapImportSettings.h"
#include "Importer/SVG/RTMSDF_SVGImportSettings.h"
#include "UObject/Object.h"
#include "RTMSDFConfig.generated.h"

UCLASS(Config=RTMSDF, DefaultConfig)
class URTMSDFConfig : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category="SVG File Rules", meta=(DisplayName="SVG Filename Suffix"))
	FString SVGFilenameSuffix = "_SDF";

	UPROPERTY(Config, EditAnywhere, Category="SVG Default Import Settings")
	TEnumAsByte<TextureGroup> SVGTextureGroup = TEXTUREGROUP_UI;

	UPROPERTY(Config, EditAnywhere, Category="SVG Default Import Settings", meta=(FullyExpand=true, DisplayName = "Default SVG Import Settings"))
	FRTMSDF_SVGImportSettings DefaultSVGImportSettings;

	UPROPERTY(Config, EditAnywhere, Category="Bitmap File Rules", meta=(DisplayName="Bitmap Filename Suffix"))
	FString BitmapFilenameSuffix = "_SDF";

	UPROPERTY(Config, EditAnywhere, Category="Bitmap Default Import Settings", meta=(FullyExpand=true, DisplayName = "Default SVG Import Settings"))
	TEnumAsByte<TextureGroup> BitmapTextureGroup = TEXTUREGROUP_UI;

	UPROPERTY(Config, EditAnywhere, Category="Bitmap Default Import Settings", meta=(FullyExpand=true, DisplayName = "Default SVG Import Settings"))
	FRTMSDF_BitmapImportSettings DefaultBitmapImportSettings;
};
