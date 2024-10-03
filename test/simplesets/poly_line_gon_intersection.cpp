#include "../catch.hpp"
#include "cartocrow/simplesets/helpers/poly_line_gon_intersection.h"
#include "cartocrow/simplesets/helpers/cs_curve_helpers.h"
#include "cartocrow/simplesets/helpers/arrangement_helpers.h"
#include "cartocrow/renderer/ipe_renderer.h"

using namespace cartocrow;
using namespace cartocrow::renderer;
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

TEST_CASE("Boundary overlap") {
	X_monotone_curve_2 xm_curve({-2, 0}, {2, 0});
	std::vector xm_curves{xm_curve};
	CSPolyline polyline(xm_curves.begin(), xm_curves.end());
	std::vector<X_monotone_curve_2> xm_curves_pgn{{{-4, 0}, {4, 0}}, {{4, 0}, {4, 2}}, {{4, 2}, {-4, 2}}, {{-4, 2}, {-4, 0}}};
	CSPolygon polygon(xm_curves_pgn.begin(), xm_curves_pgn.end());
	auto result1 = intersection(polyline, polygon, false);
	auto result2 = intersection(polyline, polygon, true);
	CHECK(result1.empty());
	CHECK(!result2.empty());
	CHECK(result2[0].curves_begin()->source() == OneRootPoint(-2, 0));
	CHECK(result2[0].curves_begin()->target() == OneRootPoint(2, 0));
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
	auto r1 = std::find_if(result.begin(), result.end(), [half](const CSPolyline& pl) {
	    OneRootNumber negHalfSqrt2(0, -half, 2);
		return pl.curves_begin()->source() == OneRootPoint(negHalfSqrt2, negHalfSqrt2);
	});
	CHECK(r1 != result.end());
	CHECK(r1->size() == 2);
	CHECK((++(r1->curves_begin()))->target() == OneRootPoint(0, -1));
	auto r2 = std::find_if(result.begin(), result.end(), [half](const CSPolyline& pl) {
	  return pl.curves_begin()->source() == OneRootPoint(1, 0);
	});
	CHECK(r2 != result.end());
	CHECK(r2->size() == 1);
	CHECK(r2->curves_begin()->target() == OneRootPoint(half, 0));
}

// Test case in https://github.com/CGAL/cgal/issues/8468
TEST_CASE("Poly-circular-arcs that partially overlap") {
	auto r = 5.204 * 3;
	auto r_ = 5.204 * 3 * 0.675;
	auto r2 = r * r;
	auto r2_ = r_ * r_;
	Circle<Exact> c1({2597.9, -364.3}, r2, CGAL::CLOCKWISE);
	Circle<Exact> c2({2609.2, -342.6}, r2, CGAL::CLOCKWISE);
	Circle<Exact> c2_({2609.2, -342.6}, r2_, CGAL::CLOCKWISE);

	auto getIntersections = [](const Circle<Exact>& one, const Circle<Exact>& two) {
	 CSArrangement arr;
	 CGAL::insert(arr, one);
	 CGAL::insert(arr, two);
	 std::vector<OneRootPoint> intersectionPoints;
	 for (auto vit = arr.vertices_begin(); vit != arr.vertices_end(); ++vit) {
	 	if (vit->degree() == 4) {
	 		intersectionPoints.push_back(vit->point());
	 	}
	 }
	 std::sort(intersectionPoints.begin(), intersectionPoints.end(), [](const OneRootPoint& p1, const OneRootPoint& p2) { return p1.x() < p2.x(); });
	 assert(intersectionPoints.size() == 2);
	 return intersectionPoints;
	};

	auto isp12 = getIntersections(c1, c2);

	Curve_2 arc1(c1, isp12[0], isp12[1]);
	Curve_2 arc2(c2, isp12[1], isp12[0]);
	std::vector<Curve_2> pgnArcs({arc1, arc2});

	PolyCSTraits traits;
	auto construct = traits.construct_curve_2_object();
	CSPolycurve pgnCurve(pgnArcs.begin(), pgnArcs.end());

	CSArrangement arr;
	CGAL::insert(arr, c1);
	CGAL::insert(arr, c2);
	CGAL::insert(arr, c2_);

	CSArrangement::Face_handle fh;
	for (auto eit = arr.halfedges_begin(); eit != arr.halfedges_end(); ++eit) {
		if (eit->source()->point() == isp12[0]) {
			fh = eit->face();
		}
	}

	std::vector<X_monotone_curve_2> xm_curves;
	curvesToXMonotoneCurves(pgnArcs.begin(), pgnArcs.end(), std::back_inserter(xm_curves));
	CSPolygon pgn(xm_curves.begin(), xm_curves.end());

	auto plnPoly = ccb_to_polygon<CSTraits>(fh->outer_ccb());
	CSPolyline pln(plnPoly.curves_begin(), plnPoly.curves_end());
	intersection(pln, pgn, true);
}

// todo: test difference and holes
