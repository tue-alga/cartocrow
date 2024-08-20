#ifndef CARTOCROW_POLY_LINE_GON_INTERSECTION_H
#define CARTOCROW_POLY_LINE_GON_INTERSECTION_H

#include "types.h"

namespace cartocrow::simplesets {
// todo: edge case where edges of dilated patterns overlap, so a half-edge may have multiple origins.
struct HalfEdgePolylineData {
	bool of_polyline = false;
};

using Arr = CGAL::Arrangement_2<CSTraits, CGAL::Arr_extended_dcel<CSTraits, std::monostate, HalfEdgePolylineData, std::monostate>>;

std::vector<CSPolyline> poly_line_gon_intersection(CSPolygon gon, CSPolyline line);
}

#endif //CARTOCROW_POLY_LINE_GON_INTERSECTION_H
