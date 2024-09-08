#include "../catch.hpp"
#include "cartocrow/simplesets/helpers/poly_line_gon_intersection.h"

using namespace cartocrow;
using namespace cartocrow::simplesets;

TEST_CASE("Intersection lies in polygon and has correct orientation") {
	X_monotone_curve_2 xm_curve({-2, 0}, {2, 0});
	std::vector xm_curves{xm_curve};
	CSPolyline polyline(xm_curves.begin(), xm_curves.end());
	CSPolygon disk = circleToCSPolygon({{0, 0}, 1});
	auto result = intersection(polyline, disk, false);
	CHECK(result.size() == 1);
	CHECK(result[0].size() == 1);
	CHECK(result[0].curves_begin()->source() == OneRootPoint(-1, 0));
	CHECK(result[0].curves_begin()->target() == OneRootPoint(1, 0));
}

TEST_CASE("Multiple and connected parts of intersection") {
	Number<Exact> half = 1;
	half /= 2;
	std::vector<Point<Exact>> points({{-1, -1}, {0, 0}, {0, -1 - half}, {1 + half, -1 - half}, {1 + half, 0}, {half, 0}});
	Polyline<Exact> pl(points.begin(), points.end());
	CSPolyline polyline = polylineToCSPolyline(pl);
	CSPolygon disk = circleToCSPolygon({{0, 0}, 1});
	auto result = intersection(polyline, disk, false);
	CHECK(result.size() == 2);
	CHECK(result[0].size() == 2);
	OneRootNumber negHalfSqrt2(0, -half, 2);
	CHECK(result[0].curves_begin()->source() == OneRootPoint(negHalfSqrt2, negHalfSqrt2));
	CHECK((++result[0].curves_begin())->target() == OneRootPoint(0, -1));
	CHECK(result[1].size() == 1);
	CHECK(result[1].curves_begin()->source() == OneRootPoint(1, 0));
	CHECK(result[1].curves_begin()->target() == OneRootPoint(half, 0));
}

// todo: test difference, overlapping edges, and holes
