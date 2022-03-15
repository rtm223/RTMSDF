// Copyright (c) Richard Meredith AB. All Rights Reserved

#include "RTMSDF_ThumbnailRenderer.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "Importer/Bitmap/RTMSDF_BitmapImportAssetData.h"
#include "Importer/SVG/RTMSDF_SVGImportAssetData.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ThumbnailRendering/ThumbnailManager.h"

URTMSDF_ThumbnailRenderer::URTMSDF_ThumbnailRenderer()
{
	SDFThumbnailMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/RTMSDF/Editor/M_RTMSDF_Thumbnail"), nullptr);

	// TODO - this should be using a UMaterialInstanceConstant with static switch parameters here, but I couldn't work out how to
}

void URTMSDF_ThumbnailRenderer::PostInitProperties()
{
	Super::PostInitProperties();

	// Get that to work, so it seems easier to have materialInstances in assets and read them in. Not ideal, but nvm
	//		auto materialInstance = NewObject<UMaterialInstanceConstant>(GetTransientPackage());
	//		materialInstance->SetParentEditorOnly(SDFThumbnailMaterial);
	//		// Set Static Switches 
	//		materialInstance->UpdateStaticPermutation(); // maybe
	//	Do all of these permutations on demand and cache them (or in the constructor TBH)
	SDFThumbnailMSDF = UMaterialInstanceDynamic::Create(LoadObject<UMaterialInterface>(nullptr, TEXT("/RTMSDF/Editor/MI_RTMSDF_Thumbnail_MSDF"), nullptr), this);
	SDFThumbnailBitmapRGBA = UMaterialInstanceDynamic::Create(LoadObject<UMaterialInterface>(nullptr, TEXT("/RTMSDF/Editor/MI_RTMSDF_Thumbnail_Bitmap_RGBA"), nullptr), this);
	SDFThumbnailBitmapRGBAPreserveRGB = UMaterialInstanceDynamic::Create(LoadObject<UMaterialInterface>(nullptr, TEXT("/RTMSDF/Editor/MI_RTMSDF_Thumbnail_Bitmap_RGBA_PreserveRGB"), nullptr), this);
	SDFThumbnailBitmapG = UMaterialInstanceDynamic::Create(LoadObject<UMaterialInterface>(nullptr, TEXT("/RTMSDF/Editor/MI_RTMSDF_Thumbnail_Bitmap_G"), nullptr), this);
}

void URTMSDF_ThumbnailRenderer::GetThumbnailSize(UObject* object, float zoom, uint32& outWidth, uint32& outHeight) const
{
	outHeight = 128 * zoom;
	outWidth = 128 * zoom;
}

void URTMSDF_ThumbnailRenderer::Draw(UObject* object, int32 x, int32 y, uint32 width, uint32 height, FRenderTarget* viewport, FCanvas* canvas, bool bAdditionalViewFamily)
{
	if(auto* texture = Cast<UTexture2D>(object))
	{
		UMaterialInstanceDynamic* materialInstanceDynamic = nullptr;
		FString label;
		bool isRGBA = texture->CompressionSettings == TC_EditorIcon;
		
		if(const auto* importData = texture->GetAssetUserData<URTMSDF_BitmapImportAssetData>())
		{
			if(isRGBA)
			{
				if(importData->ImportSettings.RGBAMode == ERTMSDF_RGBAMode::PreserveRGB)
					materialInstanceDynamic = SDFThumbnailBitmapRGBAPreserveRGB;
				else
					materialInstanceDynamic = SDFThumbnailBitmapRGBA;
			}
			else
			{
				materialInstanceDynamic = SDFThumbnailBitmapG;
			}
			label = TEXT("SDF");
		}
		if(texture->GetAssetUserData<URTMSDF_SVGImportAssetData>())
		{
			materialInstanceDynamic = isRGBA ?  SDFThumbnailMSDF : SDFThumbnailBitmapG;
			label = isRGBA ? TEXT("MSDF") : TEXT("SDF");
		}
		
		if(materialInstanceDynamic)
		{
			constexpr int32 checkerDensity = 8;
			auto& checker = UThumbnailManager::Get().CheckerboardTexture;
			canvas->DrawTile(0.0f, 0.0f, width, height, 0.0f, 0.0f, checkerDensity, checkerDensity, FLinearColor::White, checker->GetResource());

			if(isRGBA)
				materialInstanceDynamic->SetTextureParameterValue(ParamRGBATexture, texture);
			else
				materialInstanceDynamic->SetTextureParameterValue(ParamGrayscaleTexture, texture);

			const auto* materialProxy = materialInstanceDynamic->GetRenderProxy();
			FCanvasTileItem tileItem(FVector2D(x, y), materialProxy, FVector2D(width, height));
			canvas->DrawItem(tileItem);

			FIntPoint labelSize;
			StringSize(GEngine->GetSmallFont(), labelSize.X, labelSize.Y, *label);
			FVector2D size(width, height);
			FVector2D padding = labelSize / 32.0f;
			FVector2D scale = size / 64.0f;
			FCanvasTextItem TextItem(size - padding - FVector2D(labelSize.X, labelSize.Y) * scale, FText::FromString(label), GEngine->GetSmallFont(), FLinearColor::White);
			TextItem.bOutlined = true;
			TextItem.Scale = scale;
			TextItem.Draw(canvas);
			return;
		}
	}

	Super::Draw(object, x, y, width, height, viewport, canvas, bAdditionalViewFamily);
}
