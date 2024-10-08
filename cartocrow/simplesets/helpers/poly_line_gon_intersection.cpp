#include "poly_line_gon_intersection.h"

namespace cartocrow::simplesets {
std::vector<CSPolyline> intersection(const CSPolyline& line, const CSPolygon& gon, bool keepOverlap) {
	CSPolygonWithHoles withHoles(gon);
	return intersection(line, withHoles, keepOverlap);
}

std::vector<CSPolyline> difference(const CSPolyline& line, const CSPolygon& gon, bool keepOverlap) {
	CSPolygonWithHoles withHoles(gon);
	return difference(line, withHoles, keepOverlap);
}

std::vector<CSPolyline> intersection(const CSPolyline& line, const CSPolygonWithHoles& gon, bool keepOverlap) {
	std::vector<CSPolyline> polylines;
	intersection(line, gon, std::back_inserter(polylines), false, keepOverlap);
	return polylines;
}

std::vector<CSPolyline> difference(const CSPolyline& line, const CSPolygonWithHoles& gon, bool keepOverlap) {
	std::vector<CSPolyline> polylines;
	intersection(line, gon, std::back_inserter(polylines), true, keepOverlap);
	return polylines;
}
}
