// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "ThumbnailRendering/TextureThumbnailRenderer.h"
#include "RTMSDF_ThumbnailRenderer.generated.h"

/**
 * 
 */
UCLASS()
class RTMSDFEDITOR_API URTMSDF_ThumbnailRenderer : public UTextureThumbnailRenderer
{
	GENERATED_BODY()

	URTMSDF_ThumbnailRenderer();
	
	virtual void GetThumbnailSize(UObject* object, float zoom, uint32& outWidth, uint32& outHeight) const override;
	virtual void Draw(UObject* object, int32 x, int32 y, uint32 width, uint32 height, FRenderTarget* viewport, FCanvas* canvas, bool bAdditionalViewFamily) override;
	virtual void PostInitProperties() override;
private:
	UPROPERTY(Transient)
	UMaterialInterface* SDFThumbnailMaterial = nullptr;

	UPROPERTY(Transient)
	UMaterialInstanceDynamic* SDFThumbnailMSDF = nullptr;
	
	UPROPERTY(Transient)
	UMaterialInstanceDynamic* SDFThumbnailBitmapRGBA = nullptr;
	
	UPROPERTY(Transient)
	UMaterialInstanceDynamic* SDFThumbnailBitmapRGBAPreserveRGB = nullptr;
	
	UPROPERTY(Transient)
	UMaterialInstanceDynamic* SDFThumbnailBitmapG = nullptr;

	FName ParamIsMSDF = TEXT("IsMSDF");
	FName ParamIsMultichannelBitmap = TEXT("IsMSDF");
	FName ParamPreserveRGB = TEXT("PreserveRGB");
	FName ParamRGBATexture = TEXT("RGBATexture");
	FName ParamGrayscaleTexture = TEXT("GrayscaleTexture");
};
