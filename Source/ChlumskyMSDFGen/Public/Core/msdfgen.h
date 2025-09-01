
#pragma once

/*
 * MULTI-CHANNEL SIGNED DISTANCE FIELD GENERATOR v1.9 (2021-05-28)
 * ---------------------------------------------------------------
 * A utility by Viktor Chlumsky, (c) 2014 - 2021
 *
 * The technique used to generate multi-channel distance fields in this code
 * has been developed by Viktor Chlumsky in 2014 for his master's thesis,
 * "Shape Decomposition for Multi-Channel Distance Fields". It provides improved
 * quality of sharp corners in glyphs and other 2D shapes compared to monochrome
 * distance fields. To reconstruct an image of the shape, apply the median of three
 * operation on the triplet of sampled signed distance values.
 *
 */

#include "BitmapRef.hpp"
#include "generator-config.h"

#define MSDFGEN_VERSION "1.9"

namespace msdfgen
{
	struct Vector2;
	class SDFTransformation;
	class Shape;

/// Generates a conventional single-channel signed distance field.
void CHLUMSKYMSDFGEN_API generateSDF(const BitmapRef<float, 1> &output, const Shape &shape, const SDFTransformation &transformation, const GeneratorConfig &config = GeneratorConfig());

/// Generates a single-channel signed pseudo-distance field.
void CHLUMSKYMSDFGEN_API generatePSDF(const BitmapRef<float, 1> &output, const Shape &shape, const SDFTransformation &transformation, const GeneratorConfig &config = GeneratorConfig());

/// Generates a multi-channel signed distance field. Edge colors must be assigned first! (See edgeColoringSimple)
void CHLUMSKYMSDFGEN_API generateMSDF(const BitmapRef<float, 3> &output, const Shape &shape, const SDFTransformation &transformation, const MSDFGeneratorConfig &config = MSDFGeneratorConfig());

/// Generates a multi-channel signed distance field with true distance in the alpha channel. Edge colors must be assigned first.
void CHLUMSKYMSDFGEN_API generateMTSDF(const BitmapRef<float, 4> &output, const Shape &shape, const SDFTransformation &transformation, const MSDFGeneratorConfig &config = MSDFGeneratorConfig());
}
