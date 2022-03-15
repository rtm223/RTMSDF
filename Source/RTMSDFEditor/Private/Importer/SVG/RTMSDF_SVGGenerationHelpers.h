// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

enum class ERTMSDFFormat : uint8;
enum class ERTMSDFColoringMode : uint8;
enum class ERTMSDFErrorCorrectionMode : uint8;
struct FRTMSDFTextureSettingsCache;

namespace msdfgen
{
	template<typename T, int N> struct BitmapConstRef;
	struct Vector2;
	struct MSDFGeneratorConfig;
	struct ErrorCorrectionConfig;
	class Shape;
	class Projection;
}

namespace RTMSDFGenerationHelpers
{
	bool CreateShape(const uint8* buffer, const uint8* bufferEnd, msdfgen::Shape& outShape, msdfgen::Vector2& outSvgDims);
	void DoEdgeColoring(msdfgen::Shape& shape, ERTMSDFColoringMode mode, double angleThreshold, int64 seed);
	void ApplyErrorCorrectionModeTo(msdfgen::ErrorCorrectionConfig& config, ERTMSDFErrorCorrectionMode mode);
	void Generate(ERTMSDFFormat format, const msdfgen::MSDFGeneratorConfig& generatorConfig, const msdfgen::Vector2& msdfDims, const msdfgen::Shape& shape, const msdfgen::Projection& projection, double range, bool invertDistance, UTexture2D* outTexture);

	void ExtractSDFData(const msdfgen::BitmapConstRef<float, 3>& msdf, bool invertColor, uint8*& outData);
	void ExtractSDFData(const msdfgen::BitmapConstRef<float, 4>& msdf, bool invertColor, uint8*& outData);
	void ExtractSDFData(const msdfgen::BitmapConstRef<float, 1>& msdf, bool invertColor, uint8*& outData);

	void UpdateNewTextureSettings(UTexture2D* texture, const FRTMSDFTextureSettingsCache& cache, ERTMSDFFormat format);
}
