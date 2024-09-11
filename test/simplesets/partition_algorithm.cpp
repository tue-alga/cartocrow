#include "../catch.hpp"
#include "cartocrow/simplesets/partition_algorithm.h"

using namespace cartocrow;
using namespace cartocrow::simplesets;

TEST_CASE("Intersection delay") {
	GeneralSettings gs{1, 0, 0, 0};
	PartitionSettings ps{false, true, true, true, 0};
	std::vector<CatPoint> points({
	    {0, {0, 0}},
		{0, {0, 15}},
		{0, {15, 0}},
		{0, {15, 15}},
		{1, {7.5, 18}},
		{2, {-3, 15}},
	});

	Island p1({points[0], points[1]});
	Island p2({points[2], points[3]});
	Island p3({points[0], points[1], points[2], points[3]});
	CHECK(intersectionDelay(p3.catPoints(), p1, p2, p3, gs, ps) == 0);
	CHECK(abs(intersectionDelay(points, p1, p2, p3, gs, ps) - 3 / sqrt(2)) < M_EPSILON);
}