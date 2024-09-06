#ifndef CARTOCROW_POLY_LINE_GON_INTERSECTION_H
#define CARTOCROW_POLY_LINE_GON_INTERSECTION_H

#include "cartocrow/simplesets/types.h"

namespace cartocrow::simplesets {
// todo: edge case where edges of dilated patterns overlap, so a half-edge may have multiple origins.
struct HalfEdgePolylineData {
	bool of_polyline = false;
};

std::vector<CSPolyline> poly_line_gon_intersection(const CSPolygon& gon, const CSPolyline& line);
std::vector<CSPolyline> poly_line_gon_intersection(const CSPolygonWithHoles& gon, const CSPolyline& line);
}

#endif //CARTOCROW_POLY_LINE_GON_INTERSECTION_H
