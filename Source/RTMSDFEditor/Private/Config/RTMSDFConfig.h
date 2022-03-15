// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Importer/Bitmap/RTMSDF_BitmapImportSettings.h"
#include "Importer/SVG/RTMSDF_SVGImportSettings.h"
#include "UObject/Object.h"
#include "RTMSDFConfig.generated.h"

UCLASS(Config=RTMSDF, DefaultConfig)
class RTMSDFEDITOR_API URTMSDFConfig : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="SVG File Rules", Config, meta=(DisplayName="SVG Filename Suffix"))
	FString SVGFilenameSuffix = "_SDF";

	UPROPERTY(EditAnywhere, Category="SVG Default Import Settings", Config)
	TEnumAsByte<TextureGroup> SVGTextureGroup = TEXTUREGROUP_UI;
	
	UPROPERTY(EditAnywhere, Category="SVG Default Import Settings", Config, meta=(FullyExpand=true, DisplayName = "Default SVG Import Settings"))
	FRTMSDF_SVGImportSettings DefaultSVGImportSettings;

	UPROPERTY(EditAnywhere, Category="Bitmap File Rules", Config, meta=(DisplayName="Bitmap Filename Suffix"))
	FString BitmapFilenameSuffix = "_SDF";

	UPROPERTY(EditAnywhere, Category="Bitmap Default Import Settings", Config, meta=(FullyExpand=true, DisplayName = "Default SVG Import Settings"))
	TEnumAsByte<TextureGroup> BitmapTextureGroup = TEXTUREGROUP_UI;

	UPROPERTY(EditAnywhere, Category="Bitmap Default Import Settings", Config, meta=(FullyExpand=true, DisplayName = "Default SVG Import Settings"))
	FRTMSDF_BitmapImportSettings DefaultBitmapImportSettings;
};
