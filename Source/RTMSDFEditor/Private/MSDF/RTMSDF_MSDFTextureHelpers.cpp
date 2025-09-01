// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

#include "MSDF/RTMSDF_MSDFTextureHelpers.h"
#include "Importer/Common/RTMSDFTextureSettingsCache.h"
#include "Core/Bitmap.h"
#include "Generation/Common/RTMSDF_SDFFormat.h"
#include "MSDF/RTMSDF_MSDFGenerationHelpers.h"
#include "Generation/SVG/RTM_MSDFEnums.h"

namespace RTM::SDF::MSDFTextureHelpers
{
	using namespace msdfgen;

	void PopulateSDFTextureSourceData(ERTMSDF_SDFFormat format, const MSDFGeneratorConfig& generatorConfig, Vector2 sdfDims, const Shape& shape, const SDFTransformation& transformation, bool invertDistance, UTexture2D* texture)
	{
		using namespace MSDFGenerationHelpers;

		switch(format)
		{
			case ERTMSDF_SDFFormat::SingleChannel:
				{
					Bitmap<float, 1> sdf = GenerateSingleChannelSDF(generatorConfig, sdfDims, shape, transformation);
					uint8* pixelData = static_cast<uint8*>(FMemory::Malloc(sdfDims.x * sdfDims.y));

					ExtractSDFData<1, 1>(sdf, invertDistance, pixelData);
					texture->Source.Init(sdfDims.x, sdfDims.y, 1, 1, TSF_G8, pixelData);

					FMemory::Free(pixelData);
				}
				break;

			case ERTMSDF_SDFFormat::SingleChannelPseudo:
				{
					Bitmap<float, 1> sdf = GenerateSingleChannelPseudoSDF(generatorConfig, sdfDims, shape, transformation);
					uint8* pixelData = static_cast<uint8*>(FMemory::Malloc(sdfDims.x * sdfDims.y));

					ExtractSDFData<1, 1>(sdf, invertDistance, pixelData);
					texture->Source.Init(sdfDims.x, sdfDims.y, 1, 1, TSF_G8, pixelData);

					FMemory::Free(pixelData);
				}
				break;

			case ERTMSDF_SDFFormat::Multichannel:
				{
					Bitmap<float, 3> sdf = GenerateMSDF(generatorConfig, sdfDims, shape, transformation);
					uint8* pixelData = static_cast<uint8*>(FMemory::Malloc(sdfDims.x * sdfDims.y * 4));

					ExtractSDFData<3, 4>(sdf, invertDistance, pixelData);
					texture->Source.Init(sdfDims.x, sdfDims.y, 1, 1, TSF_BGRA8, pixelData);

					FMemory::Free(pixelData);
				}
				break;

			case ERTMSDF_SDFFormat::MultichannelPlusAlpha:
				{
					Bitmap<float, 4> sdf = GenerateMTSDF(generatorConfig, sdfDims, shape, transformation);
					uint8* pixelData = static_cast<uint8*>(FMemory::Malloc(sdfDims.x * sdfDims.y * 4));

					ExtractSDFData<4, 4>(sdf, invertDistance, pixelData);
					texture->Source.Init(sdfDims.x, sdfDims.y, 1, 1, TSF_BGRA8, pixelData);

					FMemory::Free(pixelData);
				}
				break;

			default:
				static_assert(static_cast<int>(ERTMSDF_SDFFormat::MAX) == 5);

				const int enumIntValue = static_cast<int>(format);
				const auto* uenumPtr = StaticEnum<ERTMSDF_MSDFErrorCorrectionMode>();
				const FString enumName = uenumPtr->GetNameStringByValue(enumIntValue);
				ensureAlwaysMsgf(false, TEXT("Invalid MSDF Format requested ('%s' - %d)- skipping"), *enumName, enumIntValue);
		}
	}

	void UpdateNewTextureSettings(UTexture2D* texture, const FRTMSDFTextureSettingsCache& cache, ERTMSDF_SDFFormat format)
	{
		cache.Restore(texture);

		// compression depends on the SDF type
		switch(format)
		{
			case ERTMSDF_SDFFormat::SingleChannel:
			case ERTMSDF_SDFFormat::SingleChannelPseudo:
				texture->CompressionSettings = TC_Grayscale;
				break;

			case ERTMSDF_SDFFormat::Multichannel:
			case ERTMSDF_SDFFormat::MultichannelPlusAlpha:
				texture->CompressionSettings = TC_EditorIcon;
				break;

			default:
				static_assert(static_cast<int>(ERTMSDF_SDFFormat::MAX) == 5);

				const int enumIntValue = static_cast<int>(format);
				const auto* uenumPtr = StaticEnum<ERTMSDF_MSDFErrorCorrectionMode>();
				const FString enumName = uenumPtr->GetNameStringByValue(enumIntValue);
				ensureAlwaysMsgf(false, TEXT("Unknown MSDF Format requested ('%s' - %d)- skipping"), *enumName, enumIntValue);

				texture->CompressionSettings = TC_EditorIcon;
				break;
		}

		// Force these Settings
		texture->SRGB = false;
		texture->bFlipGreenChannel = false;
	}
}
