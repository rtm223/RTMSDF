
#pragma once

#include <cstdlib>
#include "../core/Shape.h"

namespace msdfgen {

/// Builds a shape from an SVG path string
bool CHLUMSKYMSDFGEN_API buildShapeFromSvgPath(Shape &shape, const char *pathDef, double endpointSnapRange = 0);

/// RTM : Creates a shape from a preloaded SVG file
bool CHLUMSKYMSDFGEN_API buildShapeFromSvgFileBuffer(Shape &output, const char* file, size_t fileLength, int pathIndex = 0, Vector2 *dimensions = NULL);
	
/// Reads the first path found in the specified SVG file and stores it as a Shape in output.
bool CHLUMSKYMSDFGEN_API loadSvgShape(Shape &output, const char *filename, int pathIndex = 0, Vector2 *dimensions = NULL);

}
