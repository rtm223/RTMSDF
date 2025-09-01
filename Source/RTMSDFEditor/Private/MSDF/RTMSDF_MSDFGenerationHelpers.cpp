// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

#include "MSDF/RTMSDF_MSDFGenerationHelpers.h"
#include "Importer/Common/RTMSDFTextureSettingsCache.h"
#include "Module/RTMSDFEditor.h"
#include "ChlumskyMSDFGen/Public/Core/msdfgen.h"
#include "ChlumskyMSDFGen/Public/Ext/import-svg.h"
#include "Core/Bitmap.h"
#include "Core/edge-coloring.h"
#include "Core/SDFTransformation.h"
#include "Generation/Common/RTMSDF_SDFFormat.h"
#include "Generation/SVG/RTM_MSDFEnums.h"

namespace RTM::SDF::MSDFGenerationHelpers
{
	using namespace msdfgen;

	bool CreateShape(const uint8* svgBuffer, const uint8* bufferEnd, Shape& outShape, Shape::Bounds& outSvgBounds)
	{
		const size_t bufferLen = bufferEnd - svgBuffer + 1;
		return CreateShape(svgBuffer, bufferLen, outShape, outSvgBounds);
	}

	bool CreateShape(const uint8* svgBuffer, size_t bufferLen, Shape& outShape, Shape::Bounds& outBounds)
	{
		const char* input = reinterpret_cast<const char*>(svgBuffer);
		const int shapeParseResult = parseSvgShape(outShape, outBounds, input, bufferLen);

		const bool success = 0 != (shapeParseResult & SVG_IMPORT_SUCCESS_FLAG);
		if(ensureAlways(success))
		{
			outShape.normalize();
			outShape.inverseYAxis = !outShape.inverseYAxis;
		}

		return success;
	}

	void DoEdgeColoring(Shape& shape, ERTMSDF_MSDFColoringMode mode, double angleThreshold, int64 seed)
	{
		switch(mode)
		{
			case ERTMSDF_MSDFColoringMode::Simple:
				DoEdgeColoringSimple(shape, angleThreshold, seed);
				break;

			case ERTMSDF_MSDFColoringMode::InkTrap:
				DoEdgeColoringInkTrap(shape, angleThreshold, seed);
				break;

			case ERTMSDF_MSDFColoringMode::Distance:
				DoEdgeColoringDistance(shape, angleThreshold, seed);
				break;

			default:
				static_assert(static_cast<int>(ERTMSDF_MSDFColoringMode::MAX) == 4);

				const int enumIntValue = static_cast<int>(mode);
				const auto* uenumPtr = StaticEnum<ERTMSDF_MSDFColoringMode>();
				const FString enumName = uenumPtr->GetNameStringByValue(enumIntValue);
				ensureAlwaysMsgf(false, TEXT("Unknown Edge Coloring Mode requested ('%s' - %d)- skipping"), *enumName, enumIntValue);
		}
	}

	void DoEdgeColoringSimple(Shape& shape, double angleThreshold, int64 seed) { edgeColoringSimple(shape, angleThreshold, seed); }
	void DoEdgeColoringInkTrap(Shape& shape, double angleThreshold, int64 seed) { edgeColoringInkTrap(shape, angleThreshold, seed); }
	void DoEdgeColoringDistance(Shape& shape, double angleThreshold, int64 seed) { edgeColoringByDistance(shape, angleThreshold, seed); }

	void ApplyErrorCorrectionModeTo(ErrorCorrectionConfig& config, ERTMSDF_MSDFErrorCorrectionMode mode)
	{
		switch(mode)
		{
			case ERTMSDF_MSDFErrorCorrectionMode::None:
				config.mode = ErrorCorrectionConfig::DISABLED;
				config.distanceCheckMode = ErrorCorrectionConfig::DO_NOT_CHECK_DISTANCE;
				break;

			case ERTMSDF_MSDFErrorCorrectionMode::EdgeOnlyBalanced:
				config.mode = ErrorCorrectionConfig::EDGE_PRIORITY;
				config.distanceCheckMode = ErrorCorrectionConfig::CHECK_DISTANCE_AT_EDGE;
				break;

			case ERTMSDF_MSDFErrorCorrectionMode::EdgeOnlyFast:
				config.mode = ErrorCorrectionConfig::EDGE_ONLY;
				config.distanceCheckMode = ErrorCorrectionConfig::DO_NOT_CHECK_DISTANCE;
				break;

			case ERTMSDF_MSDFErrorCorrectionMode::EdgeOnlyFull:
				config.mode = ErrorCorrectionConfig::EDGE_ONLY;
				config.distanceCheckMode = ErrorCorrectionConfig::ALWAYS_CHECK_DISTANCE;
				break;

			case ERTMSDF_MSDFErrorCorrectionMode::EdgePriorityFast:
				config.mode = ErrorCorrectionConfig::EDGE_PRIORITY;
				config.distanceCheckMode = ErrorCorrectionConfig::DO_NOT_CHECK_DISTANCE;
				break;

			case ERTMSDF_MSDFErrorCorrectionMode::EdgePriorityFull:
				config.mode = ErrorCorrectionConfig::EDGE_PRIORITY;
				config.distanceCheckMode = ErrorCorrectionConfig::ALWAYS_CHECK_DISTANCE;
				break;

			case ERTMSDF_MSDFErrorCorrectionMode::IndiscriminateFast:
				config.mode = ErrorCorrectionConfig::INDISCRIMINATE;
				config.distanceCheckMode = ErrorCorrectionConfig::DO_NOT_CHECK_DISTANCE;
				break;

			case ERTMSDF_MSDFErrorCorrectionMode::IndiscriminateFull:
				config.mode = ErrorCorrectionConfig::INDISCRIMINATE;
				config.distanceCheckMode = ErrorCorrectionConfig::ALWAYS_CHECK_DISTANCE;
				break;

			default:
				static_assert(static_cast<int>(ERTMSDF_MSDFErrorCorrectionMode::MAX) == 8);

				const int enumIntValue = static_cast<int>(mode);
				const auto* uenumPtr = StaticEnum<ERTMSDF_MSDFErrorCorrectionMode>();
				const FString enumName = uenumPtr->GetNameStringByValue(enumIntValue);
				ensureAlwaysMsgf(false, TEXT("Unknown Error Correction Mode requested ('%s' - %d)- skipping"), *enumName, enumIntValue);
		}
	}

	SDFTransformation CalculateTransformation(Vector2 svgSize, int sdfSize, bool scaleToFitDistance, double absoluteRange, Vector2& outSDFSize)
	{
		const double svgMinEdge = FMath::Min(svgSize.x, svgSize.y);
		const double sdfMinEdge = sdfSize;
		const double scale = sdfMinEdge / svgMinEdge;
		outSDFSize = svgSize * scale;


		// Wonder if some of this can be broken down?
		const double svgExpand = scaleToFitDistance ? (svgMinEdge * (svgMinEdge / (svgMinEdge - absoluteRange * 2.0))) - svgMinEdge : 0;
		const double expandedScale = sdfMinEdge / (svgMinEdge + svgExpand);
		const msdfgen::Projection projection(expandedScale, svgExpand * 0.5);
		const float rangeAdjustment = (svgExpand + svgMinEdge) / svgMinEdge;
		return SDFTransformation(projection, Range(-absoluteRange, absoluteRange) * rangeAdjustment);
	}

	void Generate(ERTMSDF_SDFFormat format, const MSDFGeneratorConfig& generatorConfig, Vector2 sdfDims, const Shape& shape, const SDFTransformation& transformation, bool invertDistance, UTexture2D* outTexture)
	{
		// TODO - really need to separate out the texture stuff here from the generation. It's sort of done for the individual generations, just needs some cleanup

		switch(format)
		{
			case ERTMSDF_SDFFormat::SingleChannel:
				{
					Bitmap<float, 1> sdf = GenerateSingleChannelSDF(generatorConfig, sdfDims, shape, transformation);
					uint8* pixelData = (uint8*)malloc(sdfDims.x * sdfDims.y);
					ExtractSDFData<1, 1>(sdf, invertDistance, pixelData);

					outTexture->Source.Init(sdfDims.x, sdfDims.y, 1, 1, TSF_G8, pixelData);
					free(pixelData);
				}
				break;

			case ERTMSDF_SDFFormat::SingleChannelPseudo:
				{
					Bitmap<float, 1> sdf = GenerateSingleChannelPseudoSDF(generatorConfig, sdfDims, shape, transformation);
					uint8* pixelData = (uint8*)malloc(sdfDims.x * sdfDims.y);
					ExtractSDFData<1, 1>(sdf, invertDistance, pixelData);

					outTexture->Source.Init(sdfDims.x, sdfDims.y, 1, 1, TSF_G8, pixelData);
					free(pixelData);
				}
				break;

			case ERTMSDF_SDFFormat::Multichannel:
				{
					Bitmap<float, 3> sdf = GenerateMSDF(generatorConfig, sdfDims, shape, transformation);
					uint8* pixelData = (uint8*)malloc(sdfDims.x * sdfDims.y * 4);
					ExtractSDFData<3, 4>(sdf, invertDistance, pixelData);

					outTexture->Source.Init(sdfDims.x, sdfDims.y, 1, 1, TSF_BGRA8, pixelData);
					free(pixelData);
				}
				break;

			case ERTMSDF_SDFFormat::MultichannelPlusAlpha:
				{
					Bitmap<float, 4> sdf = GenerateMTSDF(generatorConfig, sdfDims, shape, transformation);
					uint8* pixelData = (uint8*)malloc(sdfDims.x * sdfDims.y * 4);
					ExtractSDFData<4, 4>(sdf, invertDistance, pixelData);

					outTexture->Source.Init(sdfDims.x, sdfDims.y, 1, 1, TSF_BGRA8, pixelData);
					free(pixelData);
				}
				break;

			default:
				static_assert(static_cast<int>(ERTMSDF_SDFFormat::MAX) == 5);

				const int enumIntValue = static_cast<int>(format);
				const auto* uenumPtr = StaticEnum<ERTMSDF_MSDFErrorCorrectionMode>();
				const FString enumName = uenumPtr->GetNameStringByValue(enumIntValue);
				ensureAlwaysMsgf(false, TEXT("Unknown MSDF Format requested ('%s' - %d)- skipping"), *enumName, enumIntValue);;
		}
	}

	Bitmap<float, 1> GenerateSingleChannelSDF(const MSDFGeneratorConfig& generatorConfig, Vector2 sdfDims, const Shape& shape, const SDFTransformation& transformation)
	{
		Bitmap<float, 1> sdf(sdfDims.x, sdfDims.y);
		generateSDF(sdf, shape, transformation, generatorConfig);
		return sdf;
	}

	Bitmap<float, 1> GenerateSingleChannelPseudoSDF(const MSDFGeneratorConfig& generatorConfig, Vector2 sdfDims, const Shape& shape, const SDFTransformation& transformation)
	{
		Bitmap<float, 1> psdf(sdfDims.x, sdfDims.y);
		generatePSDF(psdf, shape, transformation, generatorConfig);
		return psdf;
	}

	Bitmap<float, 3> GenerateMSDF(const MSDFGeneratorConfig& generatorConfig, Vector2 sdfDims, const Shape& shape, const SDFTransformation& transformation)
	{
		Bitmap<float, 3> msdf(sdfDims.x, sdfDims.y);
		generateMSDF(msdf, shape, transformation, generatorConfig);
		return msdf;
	}

	Bitmap<float, 4> GenerateMTSDF(const MSDFGeneratorConfig& generatorConfig, Vector2 sdfDims, const Shape& shape, const SDFTransformation& transformation)
	{
		Bitmap<float, 4> mtsdf(sdfDims.x, sdfDims.y);
		generateMTSDF(mtsdf, shape, transformation, generatorConfig);
		return mtsdf;
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
				texture->CompressionSettings = TC_EditorIcon;
				UE_LOG(RTMSDFEditor, Warning, TEXT("Unknown SDF Format requested - defaulting to UI texture format (No Compression)"));
				break;
		}

		// Force these Settings
		texture->SRGB = false;
		texture->bFlipGreenChannel = false;
	}
}
