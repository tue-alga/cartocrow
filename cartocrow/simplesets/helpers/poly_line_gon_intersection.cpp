#include "poly_line_gon_intersection.h"

namespace cartocrow::simplesets {
std::vector<CSPolyline> poly_line_gon_intersection(const CSPolygon& gon, const CSPolyline& line, bool keepOverlap) {
	CSPolygonWithHoles withHoles(gon);
	return poly_line_gon_intersection(withHoles, line, keepOverlap);
}

std::vector<CSPolyline> poly_line_gon_difference(const CSPolygon& gon, const CSPolyline& line, bool keepOverlap) {
	CSPolygonWithHoles withHoles(gon);
	return poly_line_gon_difference(withHoles, line, keepOverlap);
}

std::vector<CSPolyline> poly_line_gon_intersection(const CSPolygonWithHoles& gon, const CSPolyline& line, bool keepOverlap) {
	std::vector<CSPolyline> polylines;
	poly_line_gon_intersection(gon, line, std::back_inserter(polylines), false, keepOverlap);
	return polylines;
}

std::vector<CSPolyline> poly_line_gon_difference(const CSPolygonWithHoles& gon, const CSPolyline& line, bool keepOverlap) {
	std::vector<CSPolyline> polylines;
	poly_line_gon_intersection(gon, line, std::back_inserter(polylines), true, keepOverlap);
	return polylines;
}
}
