
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

#include "arithmetics.hpp"
#include "Vector2.h"
#include "Projection.h"
#include "Scanline.h"
#include "Shape.h"
#include "BitmapRef.hpp"
#include "Bitmap.h"
#include "bitmap-interpolation.hpp"
#include "pixel-conversion.hpp"
#include "edge-coloring.h"
#include "generator-config.h"
#include "msdf-error-correction.h"
#include "render-sdf.h"
#include "rasterization.h"
#include "sdf-error-estimation.h"
#include "save-bmp.h"
#include "save-tiff.h"
#include "shape-description.h"

#define MSDFGEN_VERSION "1.9"

namespace msdfgen {

/// Generates a conventional single-channel signed distance field.
void CHLUMSKYMSDFGEN_API generateSDF(const BitmapRef<float, 1> &output, const Shape &shape, const Projection &projection, double range, const GeneratorConfig &config = GeneratorConfig());

/// Generates a single-channel signed pseudo-distance field.
void CHLUMSKYMSDFGEN_API generatePseudoSDF(const BitmapRef<float, 1> &output, const Shape &shape, const Projection &projection, double range, const GeneratorConfig &config = GeneratorConfig());

/// Generates a multi-channel signed distance field. Edge colors must be assigned first! (See edgeColoringSimple)
void CHLUMSKYMSDFGEN_API generateMSDF(const BitmapRef<float, 3> &output, const Shape &shape, const Projection &projection, double range, const MSDFGeneratorConfig &config = MSDFGeneratorConfig());

/// Generates a multi-channel signed distance field with true distance in the alpha channel. Edge colors must be assigned first.
void CHLUMSKYMSDFGEN_API generateMTSDF(const BitmapRef<float, 4> &output, const Shape &shape, const Projection &projection, double range, const MSDFGeneratorConfig &config = MSDFGeneratorConfig());

// Old version of the function API's kept for backwards compatibility
void CHLUMSKYMSDFGEN_API generateSDF(const BitmapRef<float, 1> &output, const Shape &shape, double range, const Vector2 &scale, const Vector2 &translate, bool overlapSupport = true);
void CHLUMSKYMSDFGEN_API generatePseudoSDF(const BitmapRef<float, 1> &output, const Shape &shape, double range, const Vector2 &scale, const Vector2 &translate, bool overlapSupport = true);
void CHLUMSKYMSDFGEN_API generateMSDF(const BitmapRef<float, 3> &output, const Shape &shape, double range, const Vector2 &scale, const Vector2 &translate, const ErrorCorrectionConfig &errorCorrectionConfig = ErrorCorrectionConfig(), bool overlapSupport = true);
void CHLUMSKYMSDFGEN_API generateMTSDF(const BitmapRef<float, 4> &output, const Shape &shape, double range, const Vector2 &scale, const Vector2 &translate, const ErrorCorrectionConfig &errorCorrectionConfig = ErrorCorrectionConfig(), bool overlapSupport = true);

// Original simpler versions of the previous functions, which work well under normal circumstances, but cannot deal with overlapping contours.
void CHLUMSKYMSDFGEN_API generateSDF_legacy(const BitmapRef<float, 1> &output, const Shape &shape, double range, const Vector2 &scale, const Vector2 &translate);
void CHLUMSKYMSDFGEN_API generatePseudoSDF_legacy(const BitmapRef<float, 1> &output, const Shape &shape, double range, const Vector2 &scale, const Vector2 &translate);
void CHLUMSKYMSDFGEN_API generateMSDF_legacy(const BitmapRef<float, 3> &output, const Shape &shape, double range, const Vector2 &scale, const Vector2 &translate, ErrorCorrectionConfig errorCorrectionConfig = ErrorCorrectionConfig());
void CHLUMSKYMSDFGEN_API generateMTSDF_legacy(const BitmapRef<float, 4> &output, const Shape &shape, double range, const Vector2 &scale, const Vector2 &translate, ErrorCorrectionConfig errorCorrectionConfig = ErrorCorrectionConfig());

}
