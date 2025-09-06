// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Generation/Bitmap/RTMSDF_BitmapGenerationSettings.h"
#include "Generation/SVG/RTMSDF_SVGGenerationSettings.h"
#include "UObject/Object.h"
#include "Engine/TextureDefines.h"
#include "RTMSDF_ProjectSettings.generated.h"

enum TextureGroup : int;

UCLASS(Config=RTMSDF, DefaultConfig)
class RTMSDF_API URTMSDF_ProjectSettings : public UObject
{
	GENERATED_BODY()

public:
	URTMSDF_ProjectSettings();

	// Selected Suffix to interpret svg files as SDF
	UPROPERTY(Config, EditAnywhere, Category="File Rules", meta=(DisplayName="SVG Filename Suffix"))
	FString SVGFilenameSuffix = "_SDF";

	// Selected suffix to interpret bitmap (png, tiff, psd etc.) files as SDF
	UPROPERTY(Config, EditAnywhere, Category="File Rules", meta=(DisplayName="Bitmap Filename Suffix"))
	FString BitmapFilenameSuffix = "_SDF";

	// Default Group applied to SDF textures that are imported from SVGs
	UPROPERTY(Config, EditAnywhere, Category="SVG Default Import Settings", meta=(DisplayName="SVG Texture Group"))
	TEnumAsByte<TextureGroup> SVGTextureGroup = TEXTUREGROUP_UI;

	UPROPERTY(Config, EditAnywhere, Category="SVG Default Import Settings", meta=(FullyExpand=true, DisplayName = "Default SVG Import Settings"))
	FRTMSDF_SVGGenerationSettings DefaultSVGImportSettings;

	UPROPERTY(Config, EditAnywhere, Category="Bitmap Default Import Settings", meta=(FullyExpand=true))
	TEnumAsByte<TextureGroup> BitmapTextureGroup = TEXTUREGROUP_UI;

	UPROPERTY(Config, EditAnywhere, Category="Bitmap Default Import Settings|Single Channel", meta=(FullyExpand=true, DisplayName = "Default Bitmap Import Settings (single channel)"))
	FRTMSDF_BitmapGenerationSettings DefaultBitmapImportSettings_SingleChannel;

	UPROPERTY(Config, EditAnywhere, Category="Bitmap Default Import Settings|Multi Channel", meta=(FullyExpand=true, DisplayName = "Default Bitmap Import Settings (multi channel)"))
	FRTMSDF_BitmapGenerationSettings DefaultBitmapImportSettings_MultiChannel;

protected:
	virtual void PostInitProperties() override;
};
