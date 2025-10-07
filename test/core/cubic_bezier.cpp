#include "../catch.hpp"

#include "cartocrow/core/cubic_bezier.h"

using namespace cartocrow;

TEST_CASE("Cubic Bézier curve area") {
	CubicBezierCurve curve({0, 0}, {1, 0}, {1, 1}, {0, 0});
	auto pl = curve.polyline(10000);
	Polygon<Inexact> polygon(pl.vertices_begin(), pl.vertices_end());
	CHECK(abs(curve.signedArea() - polygon.area()) < M_EPSILON);
}

TEST_CASE("Cubic Bézier spline area") {
	std::vector<Point<Inexact>> pts({{0, 0}, {1, 2}, {2, 4}, {1, 5}, {-1, 4}, {-3, 4}, {-4, 3}, {-4, 1}, {-3, 0}, {0, 0}});
	CubicBezierSpline spline(pts.begin(), pts.end());
	auto pl = spline.polyline(10000);
	Polygon<Inexact> polygon(pl.vertices_begin(), pl.vertices_end());
	CHECK(abs(spline.signedArea() - polygon.area()) < M_EPSILON);
}
