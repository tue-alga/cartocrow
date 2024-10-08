#include "types.h"

namespace cartocrow::simplesets {
std::vector<Point<Exact>> makeExact(const std::vector<Point<Inexact>>& points) {
	std::vector<Point<Exact>> exact_points;
	std::transform(points.begin(), points.end(), std::back_inserter(exact_points),
	               [](const Point<Inexact>& pt) { return makeExact(pt); });
	return exact_points;
}

Point<Exact> makeExact(const Point<Inexact>& point) {
	return {point.x(), point.y()};
}

Circle<Exact> makeExact(const Circle<Inexact>& circle) {
	return {makeExact(circle.center()), circle.squared_radius()};
}

Polygon<Exact> makeExact(const Polygon<Inexact>& polygon) {
	std::vector<Point<Exact>> exact_points;
	std::transform(polygon.vertices_begin(), polygon.vertices_end(), std::back_inserter(exact_points),
	               [](const Point<Inexact>& pt) { return makeExact(pt); });
	return {exact_points.begin(), exact_points.end()};
}

Point<Inexact> approximateAlgebraic(const CSTraits::Point_2& algebraic_point) {
	return {CGAL::to_double(algebraic_point.x()), CGAL::to_double(algebraic_point.y())};
}
}
