#include "../catch.hpp"

#include <cmath>
#include <ctime>

#include "cartocrow/core/core.h"
#include "cartocrow/flow_map/polar_segment.h"

using namespace cartocrow;
using namespace cartocrow::flow_map;

TEST_CASE("Determining if polar segments are left or right lines") {
	PolarSegment s1(PolarPoint(1, 0), PolarPoint(5, 0.1));
	CHECK(!s1.isLeftLine());
	CHECK(!s1.isCollinear());
	CHECK(s1.isRightLine());

	PolarSegment s2(PolarPoint(1, 0), PolarPoint(5, -0.1));
	CHECK(s2.isLeftLine());
	CHECK(!s2.isCollinear());
	CHECK(!s2.isRightLine());

	PolarSegment s3(PolarPoint(1, 0), PolarPoint(5, 0));
	CHECK(!s3.isLeftLine());
	CHECK(s3.isCollinear());
	CHECK(!s3.isRightLine());
}
