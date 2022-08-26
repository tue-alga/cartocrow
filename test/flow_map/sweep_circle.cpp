#include "../catch.hpp"

#include <cmath>
#include <ctime>

#include "../../cartocrow/flow_map/sweep_circle.h"

using namespace cartocrow;
using namespace cartocrow::flow_map;

TEST_CASE("Creating and computing phi for a segment sweep edge shape") {
	SweepEdgeShape edge(SweepEdgeShape::Type::SEGMENT, PolarPoint(1, 0), PolarPoint(2, 0));

	CHECK(edge.nearR() == 1);
	CHECK(edge.farR() == 2);

	CHECK(edge.phiForR(1) == Approx(0));
	CHECK(edge.phiForR(1.5) == Approx(0));
	CHECK(edge.phiForR(2) == Approx(0));
}

TEST_CASE("Creating and computing phi for a spiral sweep edge shape") {
	SweepEdgeShape edge(SweepEdgeShape::Type::RIGHT_SPIRAL, PolarPoint(1, M_PI / 2),
	                    PolarPoint(2, 0));

	CHECK(edge.nearR() == 1);
	CHECK(edge.farR() == 2);

	CHECK(edge.phiForR(1) == Approx(M_PI / 2));
	CHECK(edge.phiForR(std::sqrt(2)) == Approx(M_PI / 4));
	CHECK(edge.phiForR(2) == Approx(0));
}

TEST_CASE("Creating a sweep circle") {
	SweepCircle circle;
	CHECK(circle.r() == 0);
	CHECK(circle.isValid());
	CHECK(circle.intervalCount() == 1);

	circle.grow(1);
	CHECK(circle.r() == 1);
	CHECK(circle.isValid());
	CHECK(circle.intervalCount() == 1);
}

TEST_CASE("Splitting, switching and merging in a sweep circle") {
	SweepCircle circle;
	circle.grow(1);
	circle.print();

	SweepInterval* i1 = circle.intervalAt(M_PI / 2);
	REQUIRE(i1 != nullptr);
	SweepEdgeShape s1(SweepEdgeShape::Type::SEGMENT, PolarPoint(1, M_PI / 2),
	                  PolarPoint(2, M_PI / 4));
	SweepEdgeShape s2(SweepEdgeShape::Type::SEGMENT, PolarPoint(1, M_PI / 2),
	                  PolarPoint(3, 3 * M_PI / 4));
	SweepCircle::SplitResult splitResult = circle.splitFromInterval(i1, s1, s2);
	splitResult.middleInterval->setType(SweepInterval::Type::OBSTACLE);
	CHECK(circle.isValid());
	CHECK(circle.intervalCount() == 3);
	circle.print();

	CHECK(splitResult.leftInterval->nextBoundary() == nullptr);
	CHECK(splitResult.leftInterval->previousBoundary() == splitResult.leftEdge);
	CHECK(splitResult.leftEdge->nextInterval() == splitResult.leftInterval);
	CHECK(splitResult.leftEdge->previousInterval() == splitResult.middleInterval);
	CHECK(splitResult.middleInterval->nextBoundary() == splitResult.leftEdge);
	CHECK(splitResult.middleInterval->previousBoundary() == splitResult.rightEdge);
	CHECK(splitResult.rightEdge->nextInterval() == splitResult.middleInterval);
	CHECK(splitResult.rightEdge->previousInterval() == splitResult.rightInterval);
	CHECK(splitResult.rightInterval->nextBoundary() == splitResult.rightEdge);
	CHECK(splitResult.rightInterval->previousBoundary() == nullptr);

	CHECK(splitResult.rightEdge->shape() == s1);
	CHECK(splitResult.leftEdge->shape() == s2);

	circle.grow(1.5);
	CHECK(circle.isValid());
	CHECK(circle.intervalCount() == 3);
	circle.print();

	circle.grow(2);
	CHECK(circle.isValid());
	CHECK(circle.intervalCount() == 3);
	circle.print();

	SweepEdgeShape s3(SweepEdgeShape::Type::SEGMENT, PolarPoint(2, M_PI / 4),
	                  PolarPoint(3, 3 * M_PI / 4));
	SweepCircle::SwitchResult switchResult = circle.switchEdge(splitResult.rightEdge, s3);
	CHECK(circle.isValid());
	CHECK(circle.intervalCount() == 3);
	circle.print();

	circle.grow(2.5);
	CHECK(circle.isValid());
	CHECK(circle.intervalCount() == 3);
	circle.print();

	circle.grow(3);
	CHECK(circle.isValid());
	CHECK(circle.intervalCount() == 3);
	circle.print();

	SweepCircle::MergeResult mergeResult =
	    circle.mergeToInterval(switchResult.newEdge, splitResult.leftEdge);
	mergeResult.mergedInterval->setType(SweepInterval::Type::REACHABLE);
	CHECK(circle.isValid());
	CHECK(circle.intervalCount() == 1);
	circle.print();

	circle.grow(3.5);
	CHECK(circle.isValid());
	CHECK(circle.intervalCount() == 1);
	circle.print();
}

TEST_CASE("Querying a sweep circle for intervals and edges") {
	SweepCircle circle;
	circle.grow(1);

	SECTION("sweep circle with a single interval") {
		SweepInterval* i1 = circle.intervalAt(0);
		CHECK(i1 != nullptr);
		CHECK(circle.edgeAt(0) == nullptr);
		SweepInterval* i2 = circle.intervalAt(M_PI);
		CHECK(i2 != nullptr);
		CHECK(i1 == i2);
		CHECK(circle.edgeAt(M_PI) == nullptr);
	}

	SECTION("sweep circle with three intervals") {
		SweepInterval* i = circle.intervalAt(M_PI / 2);
		SweepEdgeShape s1(SweepEdgeShape::Type::SEGMENT, PolarPoint(1, M_PI / 2),
		                  PolarPoint(2, M_PI / 4));
		SweepEdgeShape s2(SweepEdgeShape::Type::SEGMENT, PolarPoint(1, M_PI / 2),
		                  PolarPoint(2, 3 * M_PI / 4));
		SweepCircle::SplitResult result = circle.splitFromInterval(i, s1, s2);
		circle.grow(1.5);
		CHECK(circle.intervalAt(M_PI / 4) == result.rightInterval);
		CHECK(circle.intervalAt(M_PI / 2) == result.middleInterval);
		CHECK(circle.intervalAt(3 * M_PI / 4) == result.leftInterval);
	}
}
