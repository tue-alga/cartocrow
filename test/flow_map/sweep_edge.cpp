#include "../catch.hpp"

#include "cartocrow/flow_map/sweep_edge.h"

using namespace cartocrow;
using namespace cartocrow::flow_map;

TEST_CASE("Computing tangent angles") {
	SECTION("segments") {
		CHECK(SweepEdgeShape(PolarPoint(1, 0), PolarPoint(std::sqrt(2), M_PI / 4))
		          .tangentAngleForR(1) == Approx(M_PI / 2));
		CHECK(SweepEdgeShape(PolarPoint(1, M_PI / 2), PolarPoint(std::sqrt(2), M_PI / 4))
		          .tangentAngleForR(1) == Approx(0));
		CHECK(SweepEdgeShape(PolarPoint(1, M_PI / 2), PolarPoint(std::sqrt(2), 3 * M_PI / 4))
		          .tangentAngleForR(1) == Approx(M_PI));
		CHECK(SweepEdgeShape(PolarPoint(1, M_PI), PolarPoint(std::sqrt(2), 3 * M_PI / 4))
		          .tangentAngleForR(1) == Approx(M_PI / 2));
	}
	SECTION("inwards segments") {
		CHECK(SweepEdgeShape(PolarPoint(std::sqrt(2), M_PI / 4), PolarPoint(1, 0))
		          .tangentAngleForR(1) == Approx(M_PI / 2));
		CHECK(SweepEdgeShape(PolarPoint(std::sqrt(2), M_PI / 4), PolarPoint(1, M_PI / 2))
		          .tangentAngleForR(1) == Approx(0));
		CHECK(SweepEdgeShape(PolarPoint(std::sqrt(2), 3 * M_PI / 4), PolarPoint(1, M_PI / 2))
		          .tangentAngleForR(1) == Approx(M_PI));
		CHECK(SweepEdgeShape(PolarPoint(std::sqrt(2), 3 * M_PI / 4), PolarPoint(1, M_PI))
		          .tangentAngleForR(1) == Approx(M_PI / 2));
	}
	SECTION("spirals") {
		CHECK(SweepEdgeShape(SweepEdgeShape::Type::LEFT_SPIRAL, PolarPoint(1, M_PI / 4), M_PI / 8)
		          .tangentAngleForR(1) == Approx(3 * M_PI / 8));
		CHECK(SweepEdgeShape(SweepEdgeShape::Type::RIGHT_SPIRAL, PolarPoint(1, M_PI / 4), M_PI / 8)
		          .tangentAngleForR(1) == Approx(M_PI / 8));
	}
}

TEST_CASE("Checking if shapes depart to the left") {
	SECTION("segments to the right") {
		SweepEdgeShape e1(PolarPoint(1, 0), PolarPoint(2, M_PI / 4));
		SweepEdgeShape e2(PolarPoint(1, 0), PolarPoint(2, -M_PI / 4));
		CHECK(e1.departsOutwardsToLeftOf(1, e2));
		CHECK(!e2.departsOutwardsToLeftOf(1, e1));
	}
	SECTION("segments to the top") {
		SweepEdgeShape e1(PolarPoint(1, M_PI / 2), PolarPoint(2, 3 * M_PI / 4));
		SweepEdgeShape e2(PolarPoint(1, M_PI / 2), PolarPoint(2, M_PI / 4));
		CHECK(e1.departsOutwardsToLeftOf(1, e2));
		CHECK(!e2.departsOutwardsToLeftOf(1, e1));
	}
	SECTION("segments to the left") {
		SweepEdgeShape e1(PolarPoint(1, M_PI), PolarPoint(2, 5 * M_PI / 4));
		SweepEdgeShape e2(PolarPoint(1, M_PI), PolarPoint(2, 3 * M_PI / 4));
		CHECK(e1.departsOutwardsToLeftOf(1, e2));
		CHECK(!e2.departsOutwardsToLeftOf(1, e1));
	}
	SECTION("segments to the bottom") {
		SweepEdgeShape e1(PolarPoint(1, 3 * M_PI / 2), PolarPoint(2, 7 * M_PI / 4));
		SweepEdgeShape e2(PolarPoint(1, 3 * M_PI / 2), PolarPoint(2, 5 * M_PI / 4));
		CHECK(e1.departsOutwardsToLeftOf(1, e2));
		CHECK(!e2.departsOutwardsToLeftOf(1, e1));
	}
	SECTION("equal shapes") {
		SweepEdgeShape e(PolarPoint(1, 0), PolarPoint(2, 0));
		CHECK(!e.departsOutwardsToLeftOf(1, e));
	}
	SECTION("opposite shapes") {
		SweepEdgeShape e1(PolarPoint(1, 0), PolarPoint(2, 0));
		SweepEdgeShape e2(PolarPoint(1, 0), PolarPoint(0.5, 0));
		CHECK(!e1.departsOutwardsToLeftOf(1, e2));
		CHECK(!e2.departsOutwardsToLeftOf(1, e1));
	}
	SECTION("segment and spiral") {
		SweepEdgeShape e1(PolarPoint(1, 0), PolarPoint(2, 0));
		SweepEdgeShape e2(SweepEdgeShape::Type::LEFT_SPIRAL, PolarPoint(1, 0), 0.5);
		SweepEdgeShape e3(SweepEdgeShape::Type::RIGHT_SPIRAL, PolarPoint(1, 0), 0.5);
		CHECK(!e1.departsOutwardsToLeftOf(1, e2));
		CHECK(e2.departsOutwardsToLeftOf(1, e1));
		CHECK(e1.departsOutwardsToLeftOf(1, e3));
		CHECK(!e3.departsOutwardsToLeftOf(1, e1));
		CHECK(e2.departsOutwardsToLeftOf(1, e3));
		CHECK(!e3.departsOutwardsToLeftOf(1, e2));
	}
	SECTION("inwards segment and spiral") {
		SweepEdgeShape e1(PolarPoint(2, 0), PolarPoint(1, 0));
		SweepEdgeShape e2(SweepEdgeShape::Type::LEFT_SPIRAL, PolarPoint(1, 0), 0.5);
		SweepEdgeShape e3(SweepEdgeShape::Type::RIGHT_SPIRAL, PolarPoint(1, 0), 0.5);
		CHECK(!e1.departsOutwardsToLeftOf(1, e2));
		CHECK(e2.departsOutwardsToLeftOf(1, e1));
		CHECK(e1.departsOutwardsToLeftOf(1, e3));
		CHECK(!e3.departsOutwardsToLeftOf(1, e1));
		CHECK(e2.departsOutwardsToLeftOf(1, e3));
		CHECK(!e3.departsOutwardsToLeftOf(1, e2));
	}
}

TEST_CASE("Intersecting outwards a segment and a left spiral") {
	const SweepEdgeShape segment(PolarPoint(2, M_PI / 2), PolarPoint(Point<Inexact>(-6, 8)));
	const SweepEdgeShape spiral(SweepEdgeShape::Type::LEFT_SPIRAL, PolarPoint(2, M_PI / 2), 0.5);
	std::optional<Number<Inexact>> r;
	SECTION("segment to spiral") {
		r = segment.intersectOutwardsWith(spiral, 2);
	}
	SECTION("spiral to segment") {
		r = spiral.intersectOutwardsWith(segment, 2);
	}
	REQUIRE(r.has_value());
	CHECK(*r == Approx(4.968).margin(0.01));
	CHECK(spiral.phiForR(*r) == Approx(2.067).margin(0.01));
	CHECK(segment.phiForR(*r) == Approx(2.067).margin(0.01));
}

TEST_CASE("Intersecting inwards a segment and a left spiral") {
	const SweepEdgeShape segment(PolarPoint(2, M_PI / 2), PolarPoint(Point<Inexact>(0.5, 0.5)));
	const SweepEdgeShape spiral(SweepEdgeShape::Type::LEFT_SPIRAL, PolarPoint(2, M_PI / 2), 0.5);
	std::optional<Number<Inexact>> r;
	SECTION("segment to spiral") {
		r = segment.intersectInwardsWith(spiral, 2);
	}
	SECTION("spiral to segment") {
		r = spiral.intersectInwardsWith(segment, 2);
	}
	REQUIRE(r.has_value());
	CHECK(*r == Approx(0.949).margin(0.01));
	CHECK(spiral.phiForR(*r) == Approx(1.164).margin(0.01));
	CHECK(segment.phiForR(*r) == Approx(1.164).margin(0.01));
}

TEST_CASE("Intersecting outwards a segment and a left spiral without intersections") {
	const SweepEdgeShape segment(PolarPoint(2, M_PI / 2), PolarPoint(Point<Inexact>(6, 8)));
	const SweepEdgeShape spiral(SweepEdgeShape::Type::LEFT_SPIRAL, PolarPoint(2, M_PI / 2), 0.5);
	std::optional<Number<Inexact>> r;
	SECTION("segment to spiral") {
		r = segment.intersectOutwardsWith(spiral, 2);
	}
	SECTION("spiral to segment") {
		r = spiral.intersectOutwardsWith(segment, 2);
	}
	CHECK(!r.has_value());
}

TEST_CASE("Intersecting outwards a segment and a right spiral") {
	const SweepEdgeShape segment(PolarPoint(2, M_PI / 2), PolarPoint(Point<Inexact>(6, 8)));
	const SweepEdgeShape spiral(SweepEdgeShape::Type::RIGHT_SPIRAL, PolarPoint(2, M_PI / 2), 0.5);
	std::optional<Number<Inexact>> r;
	SECTION("segment to spiral") {
		r = segment.intersectOutwardsWith(spiral, 2);
	}
	SECTION("spiral to segment") {
		r = spiral.intersectOutwardsWith(segment, 2);
	}
	REQUIRE(r.has_value());
	CHECK(spiral.phiForR(*r) == Approx(segment.phiForR(*r)));
}

TEST_CASE("Intersecting outwards a segment and a right spiral without intersections") {
	const SweepEdgeShape segment(PolarPoint(2, M_PI / 2), PolarPoint(Point<Inexact>(-6, 8)));
	const SweepEdgeShape spiral(SweepEdgeShape::Type::RIGHT_SPIRAL, PolarPoint(2, M_PI / 2), 0.5);
	std::optional<Number<Inexact>> r;
	SECTION("segment to spiral") {
		r = segment.intersectOutwardsWith(spiral, 2);
	}
	SECTION("spiral to segment") {
		r = spiral.intersectOutwardsWith(segment, 2);
	}
	CHECK(!r.has_value());
}

TEST_CASE("Intersecting outwards a left spiral and a right spiral starting at the same point") {
	const SweepEdgeShape spiral1(SweepEdgeShape::Type::RIGHT_SPIRAL, PolarPoint(2, M_PI / 2), 0.5);
	const SweepEdgeShape spiral2(SweepEdgeShape::Type::LEFT_SPIRAL, PolarPoint(2, M_PI / 2), 0.5);
	std::optional<Number<Inexact>> r;
	SECTION("first to second") {
		r = spiral1.intersectOutwardsWith(spiral2, 2);
	}
	SECTION("second to first") {
		r = spiral2.intersectOutwardsWith(spiral1, 2);
	}
	REQUIRE(r.has_value());
	CHECK(*r > 2);
	CHECK(spiral1.phiForR(*r) == Approx(spiral2.phiForR(*r)));
}

TEST_CASE("Intersecting outwards a left spiral and a right spiral starting at different points") {
	const SweepEdgeShape spiral1(SweepEdgeShape::Type::RIGHT_SPIRAL, PolarPoint(2, 3 * M_PI / 4),
	                             0.5);
	const SweepEdgeShape spiral2(SweepEdgeShape::Type::LEFT_SPIRAL, PolarPoint(2, M_PI / 4), 0.5);
	std::optional<Number<Inexact>> r;
	SECTION("first to second") {
		r = spiral1.intersectOutwardsWith(spiral2, 2);
	}
	SECTION("second to first") {
		r = spiral2.intersectOutwardsWith(spiral1, 2);
	}
	REQUIRE(r.has_value());
	CHECK(*r > 2);
	CHECK(spiral1.phiForR(*r) == Approx(spiral2.phiForR(*r)));
}

TEST_CASE("Intersecting outwards a left spiral and a right spiral straddling the φ = 0π ray") {
	const SweepEdgeShape spiral1(SweepEdgeShape::Type::RIGHT_SPIRAL, PolarPoint(2, M_PI / 4), 0.5);
	const SweepEdgeShape spiral2(SweepEdgeShape::Type::LEFT_SPIRAL, PolarPoint(2, -M_PI / 8), 0.5);
	std::optional<Number<Inexact>> r;
	SECTION("first to second") {
		r = spiral1.intersectOutwardsWith(spiral2, 2);
	}
	SECTION("second to first") {
		r = spiral2.intersectOutwardsWith(spiral1, 2);
	}
	REQUIRE(r.has_value());
	CHECK(*r > 2);
	CHECK(spiral1.phiForR(*r) == Approx(spiral2.phiForR(*r)));
}

TEST_CASE("Intersecting outwards a left spiral and a right spiral straddling the φ = π ray") {
	const SweepEdgeShape spiral1(SweepEdgeShape::Type::RIGHT_SPIRAL, PolarPoint(2, 5 * M_PI / 4),
	                             0.5);
	const SweepEdgeShape spiral2(SweepEdgeShape::Type::LEFT_SPIRAL, PolarPoint(2, 7 * M_PI / 8), 0.5);
	std::optional<Number<Inexact>> r;
	SECTION("first to second") {
		r = spiral1.intersectOutwardsWith(spiral2, 2);
	}
	SECTION("second to first") {
		r = spiral2.intersectOutwardsWith(spiral1, 2);
	}
	REQUIRE(r.has_value());
	CHECK(*r > 2);
	CHECK(spiral1.phiForR(*r) == Approx(spiral2.phiForR(*r)));
}
