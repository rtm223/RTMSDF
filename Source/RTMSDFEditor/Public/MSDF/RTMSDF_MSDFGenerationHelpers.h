// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Core/pixel-conversion.hpp"
#include "core/Shape.h"	// Needed as we can't forward declare msdfgen::Shape::Bounds

enum class ERTMSDF_SDFFormat : uint8;
enum class ERTMSDF_MSDFColoringMode : uint8;
enum class ERTMSDF_MSDFErrorCorrectionMode : uint8;
enum class ERTMSDFDistanceMode : uint8;

struct FRTMSDFTextureSettingsCache;

namespace msdfgen
{
	struct Vector2;
	class SDFTransformation;
	struct MSDFGeneratorConfig;
	struct ErrorCorrectionConfig;

	template<typename T, int N> class Bitmap;
	template<typename T, int N> struct BitmapConstRef;
}

namespace RTM::SDF::MSDFGenerationHelpers
{
	bool CreateShape(const uint8* svgBuffer, const uint8* bufferEnd, msdfgen::Shape& outShape, msdfgen::Shape::Bounds& outSvgBounds);
	bool CreateShape(const uint8* svgBuffer, size_t bufferLen, msdfgen::Shape& outShape, msdfgen::Shape::Bounds& outSvgBounds);

	void DoEdgeColoring(msdfgen::Shape& shape, ERTMSDF_MSDFColoringMode mode, double angleThreshold, int64 seed = 0);
	void DoEdgeColoringSimple(msdfgen::Shape& shape, double angleThreshold, int64 seed = 0);
	void DoEdgeColoringInkTrap(msdfgen::Shape& shape, double angleThreshold, int64 seed = 0);
	void DoEdgeColoringDistance(msdfgen::Shape& shape, double angleThreshold, int64 seed = 0);

	void ApplyErrorCorrectionModeTo(msdfgen::ErrorCorrectionConfig& config, ERTMSDF_MSDFErrorCorrectionMode mode);

	msdfgen::SDFTransformation CalculateTransformation(msdfgen::Vector2 svgSize, int sdfSize, bool scaleToFitDistance, double absoluteRange, msdfgen::Vector2& outSDFSize);

	msdfgen::Bitmap<float, 1> GenerateSingleChannelSDF(const msdfgen::MSDFGeneratorConfig& generatorConfig, msdfgen::Vector2 msdfDims, const msdfgen::Shape& shape, const msdfgen::SDFTransformation& transformation);
	msdfgen::Bitmap<float, 1> GenerateSingleChannelPseudoSDF(const msdfgen::MSDFGeneratorConfig& generatorConfig, msdfgen::Vector2 msdfDims, const msdfgen::Shape& shape, const msdfgen::SDFTransformation& transformation);
	msdfgen::Bitmap<float, 3> GenerateMSDF(const msdfgen::MSDFGeneratorConfig& generatorConfig, msdfgen::Vector2 msdfDims, const msdfgen::Shape& shape, const msdfgen::SDFTransformation& transformation);
	msdfgen::Bitmap<float, 4> GenerateMTSDF(const msdfgen::MSDFGeneratorConfig& generatorConfig, msdfgen::Vector2 msdfDims, const msdfgen::Shape& shape, const msdfgen::SDFTransformation& transformation);

	template<int sourceWidth, int targetWidth>
	void ExtractSDFData(const msdfgen::BitmapConstRef<float, sourceWidth> sdf, bool invert, uint8*& outBuffer)
	{
		for(int y = 0; y < sdf.height; y++)
		{
			for(int x = 0, nx = sdf.width; x < nx; x++)
			{
				const int outBufferPos = (y * nx + x) * targetWidth;
				int channel = 0;
				for(channel = 0; channel < sourceWidth; ++channel)
				{
					float value = sdf(x, y)[channel];
					outBuffer[outBufferPos + channel] = msdfgen::pixelFloatToByte(invert ? value : 1.0f - value);
				}

				if(targetWidth > sourceWidth)
				{
					for(; channel < targetWidth; ++channel)
						outBuffer[outBufferPos + channel] = channel == 3 ? 255 : 0;
				}
			}
		}
	}
}
