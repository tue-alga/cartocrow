#include "../catch.hpp"

#include <cmath>
#include <ctime>

#include "../../cartocrow/core/core.h"
#include "../../cartocrow/flow_map/polar_line.h"

using namespace cartocrow;
using namespace cartocrow::flow_map;

TEST_CASE("Creating polar lines") {
	// with given foot
	PolarLine l1(PolarPoint(3, 0.5 * M_PI));
	Point<Inexact> foot = l1.foot().toCartesian();
	CHECK(foot.x() == Approx(0).margin(M_EPSILON));
	CHECK(foot.y() == Approx(3));

	// through two points
	PolarPoint p1(1, 0);
	PolarPoint p2(1, 0.5 * M_PI);
	PolarLine l2(p1, p2);
	foot = l2.foot().toCartesian();
	CHECK(foot.x() == Approx(0.5));
	CHECK(foot.y() == Approx(0.5));
}
