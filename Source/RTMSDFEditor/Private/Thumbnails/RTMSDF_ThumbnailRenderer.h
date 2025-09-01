// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "ThumbnailRendering/TextureThumbnailRenderer.h"
#include "RTMSDF_ThumbnailRenderer.generated.h"

/**
 *
 */
UCLASS()
class URTMSDF_ThumbnailRenderer : public UTextureThumbnailRenderer
{
	GENERATED_BODY()

public:
	URTMSDF_ThumbnailRenderer();

protected:
	virtual void GetThumbnailSize(UObject* object, float zoom, uint32& outWidth, uint32& outHeight) const override;
	virtual void Draw(UObject* object, int32 x, int32 y, uint32 width, uint32 height, FRenderTarget* viewport, FCanvas* canvas, bool bAdditionalViewFamily) override;
	virtual void PostInitProperties() override;

private:
	inline static const FName ParamTexture = TEXT("Texture");
	inline static const FName ParamInvertDistance = TEXT("InvertDistance");
	inline static const FName ParamSDFChannelMask = TEXT("SDFChannelMask");
};
