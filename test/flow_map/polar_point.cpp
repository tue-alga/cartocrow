#include "../catch.hpp"

#include <cmath>
#include <ctime>

#include "../../cartocrow/core/core.h"
#include "../../cartocrow/flow_map/polar_point.h"

using namespace cartocrow;
using namespace cartocrow::flow_map;

TEST_CASE("Creating polar points") {
	// at the origin
	PolarPoint p1;
	CHECK(p1.r() == 0);

	// with given r and phi
	PolarPoint p2(2, 0.5 * M_PI);
	CHECK(p2.r() == Approx(2));
	CHECK(p2.phi() == Approx(0.5 * M_PI));
	PolarPoint p3(3, 1.5 * M_PI);
	CHECK(p3.r() == Approx(3));
	CHECK(p3.phi() == Approx(-0.5 * M_PI));
	CHECK_THROWS_WITH(PolarPoint(-1, 0), "Tried to construct a polar point with r < 0");

	// at the given Cartesian coordinates
	PolarPoint p4(Point<Inexact>(1, 0));
	CHECK(p4.r() == Approx(1));
	CHECK(p4.phi() == Approx(0));
	PolarPoint p5(Point<Inexact>(0, 1));
	CHECK(p5.r() == Approx(1));
	CHECK(p5.phi() == Approx(0.5 * M_PI));
	PolarPoint p6(Point<Inexact>(-1, 0));
	CHECK(p6.r() == Approx(1));
	CHECK(p6.phi() == Approx(-M_PI));
	PolarPoint p7(Point<Inexact>(0, -1));
	CHECK(p7.r() == Approx(1));
	CHECK(p7.phi() == Approx(-0.5 * M_PI));
}

TEST_CASE("Translating polar points") {
	PolarPoint p1(1, 0.5 * M_PI);
	PolarPoint p2(p1, Vector<Inexact>(1, 0));
	CHECK(p2.r() == Approx(std::sqrt(2)));
	CHECK(p2.phi() == Approx(0.25 * M_PI));
}
