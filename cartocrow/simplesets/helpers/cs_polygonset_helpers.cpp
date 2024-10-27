#include "cs_polygonset_helpers.h"
#include "cs_polygon_helpers.h"

namespace cartocrow::simplesets {
renderer::RenderPath renderPath(const CSPolygonSet& polygonSet) {
	std::vector<CSPolygonWithHoles> withHoles;
	polygonSet.polygons_with_holes(std::back_inserter(withHoles));

	renderer::RenderPath path;
	for (const auto& h : withHoles) {
		path += renderPath(h);
	}

	return path;
}
}
