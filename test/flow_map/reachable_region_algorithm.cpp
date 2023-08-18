#include "../catch.hpp"

#include <cmath>
#include <ctime>

#include "cartocrow/flow_map/reachable_region_algorithm.h"
#include "cartocrow/flow_map/spiral_tree.h"

using namespace cartocrow;
using namespace cartocrow::flow_map;

TEST_CASE("Computing the reachable region with one obstacle") {
	auto tree = std::make_shared<SpiralTree>(Point<Inexact>(0, 0), 0.5061454830783556);
	tree->addPlace("p1", Point<Inexact>(0, 400), 1);
	Polygon<Inexact> obstacle;
	obstacle.push_back(Point<Inexact>(0, 50));
	obstacle.push_back(Point<Inexact>(8, 95));
	obstacle.push_back(Point<Inexact>(50, 140));
	obstacle.push_back(Point<Inexact>(-43, 134));
	obstacle.push_back(Point<Inexact>(-50, 100));
	tree->addObstacle(obstacle);
	ReachableRegionAlgorithm algorithm(tree);
	algorithm.run();

	// TODO add assertions
}
