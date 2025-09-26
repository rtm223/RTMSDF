// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once
#include "CoreMinimal.h"

class UTexture2D;
enum class ERTMSDF_SDFFormat : uint8;
struct FRTMSDFTextureSettingsCache;

namespace msdfgen
{
	class Shape;
	class SDFTransformation;
	struct Vector2;
	struct MSDFGeneratorConfig;
}

namespace RTM::SDF::MSDFTextureHelpers
{
	void PopulateSDFTextureSourceData(ERTMSDF_SDFFormat format, const msdfgen::MSDFGeneratorConfig& generatorConfig, msdfgen::Vector2 msdfDims, const msdfgen::Shape& shape, const msdfgen::SDFTransformation& transformation, bool invertDistance, UTexture2D* texture);
	void UpdateNewTextureSettings(UTexture2D* texture, const FRTMSDFTextureSettingsCache& cache, ERTMSDF_SDFFormat format);
}
