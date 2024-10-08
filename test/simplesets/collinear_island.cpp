#include "../catch.hpp"
#include "cartocrow/simplesets/patterns/island.h"

using namespace cartocrow;
using namespace cartocrow::simplesets;

TEST_CASE("Collinear island") {
	Island island({{0, {0, 0}}, {0, {1, 0}}, {0, {2, 0}}, {0, {3, 0}}});
	CHECK(island.coverRadius() == 0.5);
	CHECK(std::holds_alternative<Polyline<Inexact>>(island.poly()));
}