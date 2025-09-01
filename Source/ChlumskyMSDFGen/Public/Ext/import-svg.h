
#pragma once

#include "Core/Shape.h"

#ifndef MSDFGEN_DISABLE_SVG

namespace msdfgen {

extern CHLUMSKYMSDFGEN_API const int SVG_IMPORT_FAILURE;
extern CHLUMSKYMSDFGEN_API const int SVG_IMPORT_SUCCESS_FLAG;
extern CHLUMSKYMSDFGEN_API const int SVG_IMPORT_PARTIAL_FAILURE_FLAG;
extern CHLUMSKYMSDFGEN_API const int SVG_IMPORT_INCOMPLETE_FLAG;
extern CHLUMSKYMSDFGEN_API const int SVG_IMPORT_UNSUPPORTED_FEATURE_FLAG;
extern CHLUMSKYMSDFGEN_API const int SVG_IMPORT_TRANSFORMATION_IGNORED_FLAG;

/// Builds a shape from an SVG path string
bool CHLUMSKYMSDFGEN_API buildShapeFromSvgPath(Shape &shape, const char *pathDef, double endpointSnapRange = 0);

/// RTM : Creates a shape from a preloaded SVG file
int CHLUMSKYMSDFGEN_API parseSvgShape(Shape &output, Shape::Bounds &viewBox, const char *svgData, size_t svgLength);

/// Reads a single <path> element found in the specified SVG file and converts it to output Shape
bool CHLUMSKYMSDFGEN_API loadSvgShape(Shape &output, const char *filename, int pathIndex = 0, Vector2 *dimensions = NULL);

}

#endif
