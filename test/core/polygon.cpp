#include "../catch.hpp"

#include "../../cartocrow/core/polygon.h"

using namespace cartocrow;

TEST_CASE("Computing the centroid of a polygon") {
	Polygon<Exact> p;
	SECTION("counter-clockwise polygon") {
		p.push_back(Point<Exact>(0, 0));
		p.push_back(Point<Exact>(1, 0));
		p.push_back(Point<Exact>(1, 1));
		p.push_back(Point<Exact>(0, 1));
		CHECK(p.area() == 1);
	}
	SECTION("clockwise polygon") {
		p.push_back(Point<Exact>(0, 0));
		p.push_back(Point<Exact>(0, 1));
		p.push_back(Point<Exact>(1, 1));
		p.push_back(Point<Exact>(1, 0));
		CHECK(p.area() == -1);
	}
	SECTION("polygon with a single vertex") {
		p.push_back(Point<Exact>(0.5, 0.5));
		CHECK(p.area() == 0);
	}
	Point<Exact> c = centroid(p);
	CHECK(c.x() == 0.5);
	CHECK(c.y() == 0.5);
}
TEST_CASE("Computing the centroid of a zero-area polygon (should throw)") {
	Polygon<Exact> p;
	SECTION("polygon with two vertices") {
		p.push_back(Point<Exact>(0, 0));
		p.push_back(Point<Exact>(1, 1));
		CHECK(p.area() == 0);
	}
	SECTION("polygon with many vertices") {
		p.push_back(Point<Exact>(0, 0));
		p.push_back(Point<Exact>(1, 0));
		p.push_back(Point<Exact>(2, 0));
		p.push_back(Point<Exact>(2, 1));
		p.push_back(Point<Exact>(2, 2));
		p.push_back(Point<Exact>(2, 0));
		CHECK(p.area() == 0);
	}
	CHECK_THROWS_WITH(centroid(p), "Centroid cannot be computed for polygons of area 0");
}

TEST_CASE("Computing the centroid of a polygon with holes") {
	Polygon<Exact> ccwOutside;
	ccwOutside.push_back(Point<Exact>(0, 0));
	ccwOutside.push_back(Point<Exact>(1, 0));
	ccwOutside.push_back(Point<Exact>(1, 1));
	ccwOutside.push_back(Point<Exact>(0, 1));
	CHECK(ccwOutside.area() == 1);
	Polygon<Exact> cwOutside;
	cwOutside.push_back(Point<Exact>(0, 0));
	cwOutside.push_back(Point<Exact>(0, 1));
	cwOutside.push_back(Point<Exact>(1, 1));
	cwOutside.push_back(Point<Exact>(1, 0));
	CHECK(cwOutside.area() == -1);
	Polygon<Exact> ccwHole;
	ccwHole.push_back(Point<Exact>(0.25, 0.25));
	ccwHole.push_back(Point<Exact>(0.5, 0.25));
	ccwHole.push_back(Point<Exact>(0.5, 0.75));
	ccwHole.push_back(Point<Exact>(0.25, 0.75));
	CHECK(ccwHole.area() == 0.125);
	Polygon<Exact> cwHole;
	cwHole.push_back(Point<Exact>(0.25, 0.25));
	cwHole.push_back(Point<Exact>(0.25, 0.75));
	cwHole.push_back(Point<Exact>(0.5, 0.75));
	cwHole.push_back(Point<Exact>(0.5, 0.25));
	CHECK(cwHole.area() == -0.125);
	PolygonWithHoles<Exact> p;
	SECTION("counter-clockwise outside") {
		p = PolygonWithHoles<Exact>(ccwOutside);
		SECTION("counter-clockwise hole") {
			p.add_hole(ccwHole);
		}
		SECTION("clockwise hole") {
			p.add_hole(cwHole);
		}
	}
	SECTION("clockwise outside") {
		p = PolygonWithHoles<Exact>(cwOutside);
		SECTION("counter-clockwise hole") {
			p.add_hole(ccwHole);
		}
		SECTION("clockwise hole") {
			p.add_hole(cwHole);
		}
	}

	Point<Exact> c = centroid(p);
	CHECK(c.x() == Number<Exact>(29) / Number<Exact>(56));
	CHECK(c.y() == 0.5);
}
