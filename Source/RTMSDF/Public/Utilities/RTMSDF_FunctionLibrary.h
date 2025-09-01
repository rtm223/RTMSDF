// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "RTMSDF_FunctionLibrary.generated.h"

enum class ERTMSDF_SDFFormat : uint8;
UCLASS()
class RTMSDF_API URTMSDF_FunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Returns true if this texture has been imported as a signed distance field
	UFUNCTION(BlueprintPure, Category="RTM|SDF|Utilities", meta=(DisplayName = "Is SDF"))
	static bool IsSDFTexture(const UTexture2D* texture);

	// Returns the format of the signed distance field in this texture (Invalid if this is not an SDF)
	UFUNCTION(BlueprintPure, Category="RTM|SDF|Utilities", meta=(DisplayName = "Get SDF Format"))
	static ERTMSDF_SDFFormat GetSDFFormat(const UTexture2D* texture);

	// Returns the dimensions of the source asset used to generate this SDF texture, in pixels ([-1,-1] if this is not an SDF)
	UFUNCTION(BlueprintPure, Category="RTM|SDF|Utilities", meta=(DisplayName = "Get Source Dimensions"))
	static FIntPoint GetSourceDimensions(const UTexture2D* texture);

	// Returns true if the texture is a single channel SDF
	UFUNCTION(BlueprintPure, Category="RTM|SDF|Utilities", meta=(DisplayName = "Is single channel SDF"))
	static bool IsSingleChannelSDFTexture(const UTexture2D* texture);

	// Returns true if the texture is a multi-channel SDF (MSDF in RGB channels)
	UFUNCTION(BlueprintPure, Category="RTM|SDF|Utilities", meta=(DisplayName = "Is MSDF"))
	static bool IsMSDFTexture(const UTexture2D* texture);

	// Returns the UV Range of the SDF texture (-1 if this is not an SDF)
	// UV range is the range of the distance field, normalized against the shortest edge of the texture
	UFUNCTION(BlueprintPure, Category="RTM|SDF|Utilities", meta=(DisplayName = "Get SDF UV Range"))
	static float GetSDFUVRange(const UTexture2D* texture);

	// Returns true if this texture has been imported as a signed distance field
	// NOTE: At runtime this relies on the Asset Registry Tags, so Textures must be in the asset registry (by default this is true)
	UFUNCTION(BlueprintPure, Category="RTM|SDF|Utilities", meta=(DisplayName = "Is SDF (Soft Texture)"))
	static bool IsSDFSoftTexture(const TSoftObjectPtr<UTexture2D>& softTexture);

	// Returns the format of the signed distance field in this texture (Invalid if this is not an SDF)
	// NOTE: At runtime this relies on the Asset Registry Tags, so Textures must be in the asset registry (by default this is true)
	UFUNCTION(BlueprintPure, Category="RTM|SDF|Utilities", meta=(DisplayName = "Get SDF Format (Soft Texture)"))
	static ERTMSDF_SDFFormat GetSDFFormatFromSoftTexture(const TSoftObjectPtr<UTexture2D>& softTexture);

	// Returns the dimensions of the source asset used to generate this SDF texture, in pixels ([-1,-1] if this is not an SDF)
	// NOTE: At runtime this relies on the Asset Registry Tags, so Textures must be in the asset registry (by default this is true)
	UFUNCTION(BlueprintPure, Category="RTM|SDF|Utilities", meta=(DisplayName = "Get Source Dimensions (Soft Texture)"))
	static FIntPoint GetSourceDimensionsFromSoftTexture(const TSoftObjectPtr<UTexture2D>& softTexture);

	// Returns true if the texture is a single channel SDF
	// NOTE: At runtime this relies on the Asset Registry Tags, so Textures must be in the asset registry (by default this is true)
	UFUNCTION(BlueprintPure, Category="RTM|SDF|Utilities", meta=(DisplayName = "Is single channel SDF (Soft Texture)"))
	static bool IsSingleChannelSDFSoftTexture(const TSoftObjectPtr<UTexture2D>& softTexture);

	// Returns the UV Range of the SDF texture (-1 if this is not an SDF)
	// UV range is the range of the distance field, normalized against the shortest edge of the texture
	// NOTE: At runtime this relies on the Asset Registry Tags, so Textures must be in the asset registry (by default this is true)
	UFUNCTION(BlueprintPure, Category="RTM|SDF|Utilities", meta=(DisplayName = "Is MSDF (Soft Texture)"))
	static bool IsMSDFSoftTexture(const TSoftObjectPtr<UTexture2D>& softTexture);

	// Returns true if this texture has been imported as a signed distance field
	// NOTE: At runtime this relies on the Asset Registry Tags, so Textures must be in the asset registry (by default this is true)
	UFUNCTION(BlueprintPure, Category="RTM|SDF|Utilities", meta=(DisplayName = "Get SDF UV Range (Soft Texture)"))
	static float GetSDFUVRangeFromSoftTexure(const TSoftObjectPtr<UTexture2D>& softTexture);
};
