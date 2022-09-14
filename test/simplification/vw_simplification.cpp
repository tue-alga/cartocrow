#include "../catch.hpp"

#include "../../cartocrow/simplification/vw_simplification.h"

using namespace cartocrow;
using namespace cartocrow::vw_simplification;

TEST_CASE("Simplifying a list of points using Visvalingam-Whyatt") {
	auto points = std::make_shared<std::vector<Point<Exact>>>();
	points->push_back(Point<Exact>(0, 0));
	points->push_back(Point<Exact>(0.5, 0.25));
	points->push_back(Point<Exact>(1, 0));
	points->push_back(Point<Exact>(1, 1));
	points->push_back(Point<Exact>(0, 1));
	REQUIRE(points->size() == 5);

	VWSimplification vw_simplification(points);
	vw_simplification.constructAtComplexity(4);
	REQUIRE(points->size() == 4);
	CHECK((*points)[0] == Point<Exact>(0, 0));
	CHECK((*points)[1] == Point<Exact>(1, 0));
	CHECK((*points)[2] == Point<Exact>(1, 1));
	CHECK((*points)[3] == Point<Exact>(0, 1));
}
