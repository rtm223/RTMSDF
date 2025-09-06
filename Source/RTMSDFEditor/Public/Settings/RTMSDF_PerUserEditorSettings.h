// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "RTMSDF_PerUserEditorSettings.generated.h"

class UMaterialInterface;
struct FPropertyChangedEvent;

//~ NOTE: Not currently per user as I can't find a way to get the DefaultEditorPerProjectUserSettings.ini to work from a plugin
//~ TODO: Work out how UInterchangeProjectSettings manages to be per-user, despite being in the engine config
UCLASS(Config=RTMSDF, meta=(DisplayName="RTMSDF Editor Settings"))
class RTMSDFEDITOR_API URTMSDF_PerUserEditorSettings : public UObject
{
	GENERATED_BODY()

public:
	// Material to be used for SDFs with a single channel
	UPROPERTY(Config, EditAnywhere, Category="Thumbnails", meta=(DisplayName="Single Channel Material"))
	TSoftObjectPtr<UMaterialInterface> SDFThumbnailSingleChannel = nullptr;

	// Material to be used for MSDFs
	UPROPERTY(Config, EditAnywhere, Category="Thumbnails", meta=(DisplayName="MSDF Material"))
	TSoftObjectPtr<UMaterialInterface> SDFThumbnailMSDF = nullptr;

	// Material to be used for SDFs with multiple channels (i.e. with separate SDFs packed into RGBA)
	UPROPERTY(Config, EditAnywhere, Category="Thumbnails", meta=(DisplayName="Multichannel Material"))
	TSoftObjectPtr<UMaterialInterface> SDFThumbnailMultichannel = nullptr;

	// Material to be used for textures that have SDF baked into the alpha channel, but RGB channels hold original color data)
	UPROPERTY(Config, EditAnywhere, Category="Thumbnails", meta=(DisplayName="Preserve RGB Material"))
	TSoftObjectPtr<UMaterialInterface> SDFThumbnailPreserveRGB = nullptr;

	UPROPERTY(Config, EditAnywhere, Category="Thumbnails")
	bool bLabelThumbnailsAsSDF = true;

#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> SDFThumbnailSingleChannel_Inst = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> SDFThumbnailMSDF_Inst = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> SDFThumbnailMultichannel_Inst = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> SDFThumbnailPreserveRGB_Inst = nullptr;
#endif

protected:
	virtual void PostInitProperties() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent) override;
	void LoadMaterials();
};
