#include "../catch.hpp"

#include <cmath>
#include <ctime>

#include "cartocrow/flow_map/painting.h"
#include "cartocrow/flow_map/spiral_tree.h"
#include "cartocrow/flow_map/spiral_tree_obstructed_algorithm.h"

#include "cartocrow/renderer/ipe_renderer.h"

using namespace cartocrow;
using namespace cartocrow::flow_map;

TEST_CASE("Computing a spiral tree with one node") {
	auto tree = std::make_shared<SpiralTree>(Point<Inexact>(0, 0), 0.5061454830783556);
	tree->addPlace("p1", Point<Inexact>(0, 100), 1);
	CHECK(tree->nodes().size() == 2);

	int expectedNodeCount;
	SECTION("without obstacle") {
		expectedNodeCount = 2;
	}

	SECTION("with obstacle") {
		Polygon<Inexact> obstacle;
		obstacle.push_back(Point<Inexact>(-10, 50));
		obstacle.push_back(Point<Inexact>(0, 25));
		obstacle.push_back(Point<Inexact>(10, 50));
		tree->addObstacle(obstacle);
		expectedNodeCount = 4;
	}

	ReachableRegionAlgorithm algorithm(tree);
	auto reachable = algorithm.run();
	SpiralTreeObstructedAlgorithm algorithm2(tree, reachable);
	algorithm2.run();

	cartocrow::renderer::IpeRenderer renderer(algorithm2.debugPainting());
	renderer.save("/tmp/test.ipe");

	REQUIRE(tree->nodes().size() == expectedNodeCount);
}
