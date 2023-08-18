#include "../catch.hpp"

#include <cmath>
#include <ctime>

#include <QApplication>

#include "cartocrow/flow_map/spiral_tree.h"

using namespace cartocrow;
using namespace cartocrow::flow_map;

TEST_CASE("Adding an obstacle to a spiral tree") {
	auto tree = std::make_shared<SpiralTree>(Point<Inexact>(0, 0), 1.5);
	Polygon<Inexact> shape;

	SECTION("subdividing one edge") {
		shape.push_back(Point<Inexact>(-2, 4));
		shape.push_back(Point<Inexact>(2, 4));
		shape.push_back(Point<Inexact>(0, 6));
		tree->addObstacle(shape);
		REQUIRE(tree->obstacles().size() == 1);
		CHECK(tree->obstacles()[0].size() == 6); // 3 vertices added
	}
	SECTION("subdividing two edges") {
		shape.push_back(Point<Inexact>(-2, 4));
		shape.push_back(Point<Inexact>(2, 4));
		shape.push_back(Point<Inexact>(-2, 5));
		tree->addObstacle(shape);
		REQUIRE(tree->obstacles().size() == 1);
		CHECK(tree->obstacles()[0].size() == 9); // 2 * 3 vertices added
	}
}
