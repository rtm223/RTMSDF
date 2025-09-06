// Copyright (c) Richard Meredith AB. All Rights Reserved

#include "RTMSDF_ThumbnailRenderer.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "Generation/Bitmap/RTMSDF_BitmapGenerationAssetData.h"
#include "Generation/SVG/RTMSDF_SVGGenerationAssetData.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Settings/RTMSDF_PerUserEditorSettings.h"
#include "ThumbnailRendering/ThumbnailManager.h"

URTMSDF_ThumbnailRenderer::URTMSDF_ThumbnailRenderer()
{
	//SDFThumbnailMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/RTMSDF/Editor/M_RTMSDF_Thumbnail"), nullptr);

	// TODO - this should be using a UMaterialInstanceConstant with static switch parameters here, but I couldn't work out how to
}

void URTMSDF_ThumbnailRenderer::PostInitProperties()
{
	Super::PostInitProperties();
}

void URTMSDF_ThumbnailRenderer::GetThumbnailSize(UObject* object, float zoom, uint32& outWidth, uint32& outHeight) const
{
	outHeight = 128 * zoom;
	outWidth = 128 * zoom;
}

void URTMSDF_ThumbnailRenderer::Draw(UObject* object, int32 x, int32 y, uint32 width, uint32 height, FRenderTarget* viewport, FCanvas* canvas, bool bAdditionalViewFamily)
{
	const auto* settings = GetDefault<URTMSDF_PerUserEditorSettings>();

	if(auto* texture = Cast<UTexture2D>(object))
	{
		FLinearColor sdfChannelMask(1,1,1,1);
		UMaterialInterface* material = nullptr;
		FString label = TEXT("SDF");
		bool isRGBA = texture->CompressionSettings == TC_EditorIcon;
		bool invertSDF = false;
		if(const auto* importData = texture->GetAssetUserData<URTMSDF_BitmapGenerationAssetData>())
		{
			invertSDF = importData->GenerationSettings.bInvertDistance;

			if(isRGBA)
			{
				if(importData->GenerationSettings.LooksLikePreserveRGB())
				{
					material = settings->SDFThumbnailPreserveRGB_Inst;
					sdfChannelMask = FLinearColor(0,0,0,1);
				}
				else
				{
					material = settings->SDFThumbnailMultichannel_Inst;
					sdfChannelMask = FLinearColor(
						importData->GenerationSettings.GetChannelBehavior(ERTMSDF_Channels::Red) == ERTMSDF_BitmapChannelBehavior::SDF ? 1.0f : 0.0f,
						importData->GenerationSettings.GetChannelBehavior(ERTMSDF_Channels::Green) == ERTMSDF_BitmapChannelBehavior::SDF ? 1.0f : 0.0f,
						importData->GenerationSettings.GetChannelBehavior(ERTMSDF_Channels::Blue) == ERTMSDF_BitmapChannelBehavior::SDF ? 1.0f : 0.0f,
						importData->GenerationSettings.GetChannelBehavior(ERTMSDF_Channels::Alpha) == ERTMSDF_BitmapChannelBehavior::SDF ? 1.0f : 0.0f
					);
				}
			}
			else
			{
				material = settings->SDFThumbnailSingleChannel_Inst;
			}
		}
		if(const auto* importData = texture->GetAssetUserData<URTMSDF_SVGGenerationAssetData>())
		{
			invertSDF = importData->GenerationSettings.bInvertDistance;
			material = isRGBA ?  settings->SDFThumbnailMSDF_Inst : settings->SDFThumbnailSingleChannel_Inst;
			label = isRGBA ? TEXT("MSDF") : TEXT("SDF");
		}

		if(material)
		{
			constexpr int32 checkerDensity = 8;
			auto& checker = UThumbnailManager::Get().CheckerboardTexture;
			canvas->DrawTile(0.0f, 0.0f, width, height, 0.0f, 0.0f, checkerDensity, checkerDensity, FLinearColor::White, checker->GetResource());

			// TODO - maybe these should be cached out somewhere rather than thrashing the objects?
			auto* materialInstanceDynamic = UMaterialInstanceDynamic::Create(material, this);
			materialInstanceDynamic->SetTextureParameterValue(ParamTexture, texture);
			materialInstanceDynamic->SetScalarParameterValue(ParamInvertDistance, invertSDF ? 1.0f : 0.0f);
			materialInstanceDynamic->SetVectorParameterValue(ParamSDFChannelMask, sdfChannelMask);
			const auto* materialProxy = materialInstanceDynamic->GetRenderProxy();
			FCanvasTileItem tileItem(FVector2D(x, y), materialProxy, FVector2D(width, height));
			canvas->DrawItem(tileItem);

			if(settings->bLabelThumbnailsAsSDF)
			{
				FIntPoint labelSize;
				StringSize(GEngine->GetSmallFont(), labelSize.X, labelSize.Y, *label);
				FVector2D size(width, height);
				FVector2D padding = labelSize / 5.0;
				FVector2D scale = size / 96.0;
				FVector2D shadowOffset(scale * 1.5);
				FCanvasTextItem TextItem(size - padding - FVector2D(labelSize.X, labelSize.Y) * scale, FText::FromString(label), GEngine->GetSmallFont(), FLinearColor::White*2.0f);
				TextItem.bOutlined = true;
				TextItem.EnableShadow(FLinearColor(0.04f, 0.04f, 0.04f, 1.0f), shadowOffset);
				TextItem.Scale = scale;
				TextItem.Draw(canvas);
				return;
			}
		}
	}
	Super::Draw(object, x, y, width, height, viewport, canvas, bAdditionalViewFamily);
}
