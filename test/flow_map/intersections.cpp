#include "../catch.hpp"

#include <cmath>
#include <ctime>

#include "../../cartocrow/core/core.h"
#include "../../cartocrow/flow_map/intersections.h"
#include "../../cartocrow/flow_map/polar_line.h"
#include "../../cartocrow/flow_map/spiral.h"
#include "../../cartocrow/flow_map/spiral_segment.h"

using namespace cartocrow;
using namespace cartocrow::flow_map;

/// Custom matcher to check polar points.
class IsPolarCloseTo : public Catch::MatcherBase<PolarPoint> {
	const PolarPoint& m_expected;

  public:
	IsPolarCloseTo(const PolarPoint& expected) : m_expected(expected) {}
	bool match(const PolarPoint& p) const override {
		return p.r() == Approx(m_expected.r()).margin(0.0001) &&
		       (p.r() == Approx(0) || p.phi() == Approx(m_expected.phi()).margin(0.0001));
	}
	std::string describe() const override {
		std::ostringstream ss;
		ss << "is close to " << m_expected;
		return ss.str();
	}
};

TEST_CASE("Computing intersections") {
	const PolarLine line_1(PolarPoint(Point<Inexact>(11, -2)), PolarPoint(Point<Inexact>(-1, 7)));
	const PolarLine line_2(PolarPoint(Point<Inexact>(-2, -4)), PolarPoint(Point<Inexact>(1, 0)));
	const PolarLine line_3(PolarPoint(Point<Inexact>(4, -3)), PolarPoint(Point<Inexact>(0, 0)));
	const PolarLine line_4(PolarPoint(Point<Inexact>(0, 0)), PolarPoint(Point<Inexact>(4, 0)));
	const Spiral spiral_1(PolarPoint(Point<Inexact>(11, -3)), M_PI * 3.0 / 8);
	const Spiral spiral_2(PolarPoint(Point<Inexact>(11, -3)), -M_PI * 3.0 / 8);
	const Spiral spiral_3(PolarPoint(Point<Inexact>(2, 3)), M_PI_4);
	const Spiral spiral_4(PolarPoint(Point<Inexact>(-11, 3)), M_PI * 3.0 / 8);
	const Spiral spiral_5(PolarPoint(Point<Inexact>(4, -3)), 0);

	const PolarSegment line_segment_1(PolarPoint(Point<Inexact>(11, -2)),
	                                  PolarPoint(Point<Inexact>(-1, 7)));
	const PolarSegment line_segment_2(PolarPoint(Point<Inexact>(11, -2)),
	                                  PolarPoint(Point<Inexact>(7, 1)));
	const SpiralSegment spiral_segment_1(PolarPoint(Point<Inexact>(5, 5)), M_PI * 3.0 / 8, 0, 15);
	const SpiralSegment spiral_segment_2(PolarPoint(Point<Inexact>(5, 5)), M_PI * 3.0 / 8, 0, 10);
	const SpiralSegment spiral_segment_3(PolarPoint(Point<Inexact>(5, 5)), M_PI * 3.0 / 8, 6, 10);

	std::vector<PolarPoint> intersections;

	SECTION("line-line intersection") {
		intersect(line_1, line_2, intersections);
		REQUIRE(intersections.size() == 1);
		CHECK_THAT(intersections[0], IsPolarCloseTo(PolarPoint(5.0636, 0.7686)));
	}

	SECTION("line-line intersection (parallel)") {
		intersect(line_1, line_3, intersections);
		REQUIRE(intersections.size() == 0);
	}

	SECTION("line-line intersection (intersection at origin)") {
		intersect(line_3, line_4, intersections);
		REQUIRE(intersections.size() == 1);
		CHECK_THAT(intersections[0], IsPolarCloseTo(PolarPoint(0, 0)));
	}

	SECTION("spiral-spiral intersection (opposite angle)") {
		intersect(spiral_1, spiral_2, intersections);
		REQUIRE(intersections.size() == 2);
		CHECK_THAT(intersections[0], IsPolarCloseTo(PolarPoint(3.1033, 2.8753)));
		CHECK_THAT(intersections[1], IsPolarCloseTo(PolarPoint(11.4018, -0.2663)));
	}

	SECTION("spiral-spiral intersection") {
		intersect(spiral_2, spiral_3, intersections);
		REQUIRE(intersections.size() == 2);
		CHECK_THAT(intersections[0], IsPolarCloseTo(PolarPoint(1.8628, 1.6432)));
		CHECK_THAT(intersections[1], IsPolarCloseTo(PolarPoint(11.7329, -0.1971)));
	}

	SECTION("spiral-spiral intersection (equal angle)") {
		intersect(spiral_1, spiral_4, intersections);
		REQUIRE(intersections.size() == 0);
	}

	SECTION("line-spiral intersection") {
		intersect(line_1, spiral_1, intersections);
		REQUIRE(intersections.size() == 2);
		CHECK_THAT(intersections[0], IsPolarCloseTo(PolarPoint(51.0082, 2.3999)));
		CHECK_THAT(intersections[1], IsPolarCloseTo(PolarPoint(10.9538, -0.1695)));
	}

	SECTION("line-spiral intersection (one side)") {
		intersect(line_2, spiral_3, intersections);
		REQUIRE(intersections.size() == 1);
		CHECK_THAT(intersections[0], IsPolarCloseTo(PolarPoint(4.5484, 0.7505)));
	}

	SECTION("line-spiral intersection (line through origin)") {
		intersect(spiral_1, line_3, intersections);
		REQUIRE(intersections.size() == 2);
		CHECK_THAT(intersections[0], IsPolarCloseTo(PolarPoint(13.3302, -0.6435)));
		CHECK_THAT(intersections[1], IsPolarCloseTo(PolarPoint(3.6282, 2.4981)));
	}

	SECTION("line-spiral intersection (spiral has angle zero)") {
		intersect(spiral_5, line_2, intersections);
		REQUIRE(intersections.size() == 1);
		CHECK_THAT(intersections[0], IsPolarCloseTo(PolarPoint(0.8000, -0.6435)));
	}

	SECTION("line-spiral intersection (spiral has angle zero and is parallel to the line)") {
		intersect(spiral_5, line_1, intersections);
		REQUIRE(intersections.size() == 0);
	}

	SECTION("line-spiral intersection (spiral has angle zero, line through pole)") {
		intersect(spiral_5, line_4, intersections);
		REQUIRE(intersections.size() == 1);
		CHECK_THAT(intersections[0], IsPolarCloseTo(PolarPoint(0, 0)));
	}

	SECTION("line-spiral intersection (spiral has angle zero and is parallel to the line, line "
	        "through pole)") {
		intersect(spiral_5, line_3, intersections);
		REQUIRE(intersections.size() == 2);
		CHECK_THAT(intersections[0], IsPolarCloseTo(PolarPoint(0, 0)));
		CHECK_THAT(intersections[1], IsPolarCloseTo(PolarPoint(5, -0.6435)));
	}

	SECTION("line-segment intersection") {
		intersect(line_2, line_segment_1, intersections);
		REQUIRE(intersections.size() == 1);
	}

	SECTION("line-segment intersection (no intersections)") {
		intersect(line_2, line_segment_2, intersections);
		REQUIRE(intersections.size() == 0);
	}

	SECTION("line-spiral segment intersection") {
		intersect(line_1, spiral_segment_1, intersections);
		REQUIRE(intersections.size() == 2);
	}

	SECTION("line-spiral segment intersection (one intersection)") {
		intersect(line_1, spiral_segment_2, intersections);
		REQUIRE(intersections.size() == 1);
	}

	SECTION("line-spiral segment intersection (no intersections)") {
		intersect(line_1, spiral_segment_3, intersections);
		REQUIRE(intersections.size() == 0);
	}

	SECTION("segment-spiral segment intersection (no intersections)") {
		intersect(line_segment_2, spiral_segment_2, intersections);
		REQUIRE(intersections.size() == 0);
	}
}
