// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

#include "RTMSDF_SVGGenerationHelpers.h"
#include "Importer/RTMSDFTextureSettingsCache.h"
#include "RTMSDF_SVGImportSettings.h"
#include "Module/RTMSDFEditor.h"
#include "ChlumskyMSDFGen/Public/Core/msdfgen.h"
#include "ChlumskyMSDFGen/Public/Ext/import-svg.h"
#include "Engine/Texture2DArray.h"

namespace RTMSDFGenerationHelpers
{
	using namespace msdfgen;

	bool CreateShape(const uint8* buffer, const uint8* bufferEnd, Shape& outShape, Vector2& outSvgDims)
	{
		size_t bufferLen = bufferEnd - buffer + 1;
		char* input = new char[bufferLen];
		for(int i = 0; i < bufferLen; i++)
			input[i] = static_cast<char>(buffer[i]);		// TODO - memcpy here;

		bool builtShape = buildShapeFromSvgFileBuffer(outShape, input, bufferLen, 0, &outSvgDims);
		delete(input);

		if(builtShape)
		{
			outShape.normalize();
			outShape.inverseYAxis = !outShape.inverseYAxis;
		}

		return builtShape;
	}

	void DoEdgeColoring(Shape& shape, ERTMSDFColoringMode mode, double angleThreshold, int64 seed)
	{
		switch(mode)
		{
			case ERTMSDFColoringMode::Simple:
				edgeColoringSimple(shape, angleThreshold, seed);
				break;
			case ERTMSDFColoringMode::InkTrap:
				edgeColoringInkTrap(shape, angleThreshold, seed);
				break;
			case ERTMSDFColoringMode::Distance:
				edgeColoringByDistance(shape, angleThreshold, seed);
				break;
			default:
				UE_LOG(RTMSDFEditor, Warning, TEXT("Unknown Edge Coloring Mode requested - skipping"));
		}
	}

	void ApplyErrorCorrectionModeTo(ErrorCorrectionConfig& config, ERTMSDFErrorCorrectionMode mode)
	{
		switch(mode)
		{
			case ERTMSDFErrorCorrectionMode::None:
				config.mode = ErrorCorrectionConfig::DISABLED;
				config.distanceCheckMode = ErrorCorrectionConfig::DO_NOT_CHECK_DISTANCE;
				break;
			case ERTMSDFErrorCorrectionMode::EdgeOnlyBalanced:
				config.mode = ErrorCorrectionConfig::EDGE_PRIORITY;
				config.distanceCheckMode = ErrorCorrectionConfig::CHECK_DISTANCE_AT_EDGE;
				break;
			case ERTMSDFErrorCorrectionMode::EdgeOnlyFast:
				config.mode = ErrorCorrectionConfig::EDGE_ONLY;
				config.distanceCheckMode = ErrorCorrectionConfig::DO_NOT_CHECK_DISTANCE;
				break;
			case ERTMSDFErrorCorrectionMode::EdgeOnlyFull:
				config.mode = ErrorCorrectionConfig::EDGE_ONLY;
				config.distanceCheckMode = ErrorCorrectionConfig::ALWAYS_CHECK_DISTANCE;
				break;
			case ERTMSDFErrorCorrectionMode::EdgePriorityFast:
				config.mode = ErrorCorrectionConfig::EDGE_PRIORITY;
				config.distanceCheckMode = ErrorCorrectionConfig::DO_NOT_CHECK_DISTANCE;
				break;
			case ERTMSDFErrorCorrectionMode::EdgePriorityFull:
				config.mode = ErrorCorrectionConfig::EDGE_PRIORITY;
				config.distanceCheckMode = ErrorCorrectionConfig::ALWAYS_CHECK_DISTANCE;
				break;
			case ERTMSDFErrorCorrectionMode::IndiscriminateFast:
				config.mode = ErrorCorrectionConfig::INDISCRIMINATE;
				config.distanceCheckMode = ErrorCorrectionConfig::DO_NOT_CHECK_DISTANCE;
				break;
			case ERTMSDFErrorCorrectionMode::IndiscriminateFull:
				config.mode = ErrorCorrectionConfig::INDISCRIMINATE;
				config.distanceCheckMode = ErrorCorrectionConfig::ALWAYS_CHECK_DISTANCE;
				break;
			default:
				UE_LOG(RTMSDFEditor, Warning, TEXT("Unknown Error Correction Mode requested - selecting 'Default'"));
		}
	}

	void Generate(ERTMSDFFormat format, const MSDFGeneratorConfig& generatorConfig, const Vector2& msdfDims, const Shape& shape, const Projection& projection, double range, bool invertDistance, UTexture2D* outTexture)
	{
		switch(format)
		{
			case ERTMSDFFormat::SingleChannel:
				{
					Bitmap<float, 1> sdf(msdfDims.x, msdfDims.y);
					generateSDF(sdf, shape, projection, range, generatorConfig);

					uint8* pixelData = (uint8*)malloc(msdfDims.x * msdfDims.y);
					ExtractSDFData(sdf, invertDistance, pixelData);

					outTexture->Source.Init(msdfDims.x, msdfDims.y, 1, 1, TSF_G8, pixelData);
					free(pixelData);
				}
				break;
			
			case ERTMSDFFormat::SingleChannelPseudo:
				{
					Bitmap<float, 1> psdf(msdfDims.x, msdfDims.y);
					generatePseudoSDF(psdf, shape, projection, range, generatorConfig);
					uint8* pixelData = (uint8*)malloc(msdfDims.x * msdfDims.y);
					ExtractSDFData(psdf, invertDistance, pixelData);

					outTexture->Source.Init(msdfDims.x, msdfDims.y, 1, 1, TSF_G8, pixelData);
					free(pixelData);
				}
				break;

			case ERTMSDFFormat::Multichannel:
				{
					Bitmap<float, 3> msdf(msdfDims.x, msdfDims.y);
					generateMSDF(msdf, shape, projection, range, generatorConfig);

					uint8* pixelData = (uint8*)malloc(msdfDims.x * msdfDims.y * 4);
					ExtractSDFData(msdf, invertDistance, pixelData);

					outTexture->Source.Init(msdfDims.x, msdfDims.y, 1, 1, TSF_BGRA8, pixelData);
					free(pixelData);
				}
				break;

			case ERTMSDFFormat::MultichannelPlusAlpha:
				{
					Bitmap<float, 4> mtsdf(msdfDims.x, msdfDims.y);
					generateMTSDF(mtsdf, shape, projection, range, generatorConfig);
					uint8* pixelData = (uint8*)malloc(msdfDims.x * msdfDims.y * 4);
					ExtractSDFData(mtsdf, invertDistance, pixelData);

					outTexture->Source.Init(msdfDims.x, msdfDims.y, 1, 1, TSF_BGRA8, pixelData);
					free(pixelData);
				}
				break;
			default: ;
		}
	}

	void ExtractSDFData(const BitmapConstRef<float, 3>& msdf, bool invertColor, uint8*& outData)
	{
		for(int y = 0; y < msdf.height; y++)
		{
			for(int x = 0, nx = msdf.width; x < nx; x++)
			{
				int pixelPos = 4 * (y * nx + x);
				for(int c = 0; c < 3; c++)
				{
					float value = msdf(x, y)[c];
					outData[pixelPos + c] = pixelFloatToByte(invertColor ? 1.0f - value : value); // BGR
				}

				outData[pixelPos + 3] = 255; // A						
			}
		}
	}

	void ExtractSDFData(const BitmapConstRef<float, 4>& msdf, bool invertColor, uint8*& outData)
	{
		for(int y = 0; y < msdf.height; y++)
		{
			for(int x = 0, nx = msdf.width; x < nx; x++)
			{
				int pixelPos = 4 * (y * nx + x);
				for(int c = 0; c < 4; c++)
				{
					float value = msdf(x, y)[c];
					outData[pixelPos + c] = pixelFloatToByte(invertColor ? 1.0f - value : value); // BGRA
				}
			}
		}
	}

	void ExtractSDFData(const BitmapConstRef<float, 1>& msdf, bool invertColor, uint8*& outData)
	{
		for(int y = 0; y < msdf.height; y++)
		{
			for(int x = 0, nx = msdf.width; x < nx; x++)
			{
				int pixelPos = (y * nx + x);
				float value = msdf(x, y)[0];
				outData[pixelPos] = pixelFloatToByte(invertColor ? 1.0f - value : value); // A
			}
		}
	}

	void UpdateNewTextureSettings(UTexture2D* texture, const FRTMSDFTextureSettingsCache& cache, ERTMSDFFormat format)
	{
		cache.Restore(texture);

		// compression depends on the SDF type
		switch(format)
		{
			case ERTMSDFFormat::SingleChannel:
			case ERTMSDFFormat::SingleChannelPseudo:
				texture->CompressionSettings = TC_Grayscale;
				break;
			case ERTMSDFFormat::Multichannel:
			case ERTMSDFFormat::MultichannelPlusAlpha:
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
