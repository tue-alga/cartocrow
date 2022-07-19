#include "../catch.hpp"

#include "../../cartocrow/core/core.h"
#include "../../cartocrow/core/region_map.h"
#include "../../cartocrow/necklace_map/circle_necklace.h"
#include "../../cartocrow/necklace_map/necklace_map.h"
#include "../../cartocrow/necklace_map/painting.h"
#include "../../cartocrow/renderer/ipe_renderer.h"

using namespace cartocrow;
using namespace cartocrow::necklace_map;

TEST_CASE("Computing a necklace map") {
	std::shared_ptr<RegionMap> regions = std::make_shared<RegionMap>(
	    ipeToRegionMap(std::filesystem::path("data/test_region_map.ipe")));
	NecklaceMap map(regions);
	auto necklace = map.addNecklace(
	    std::make_unique<CircleNecklace>(Circle<Inexact>(Point<Inexact>(64, 32), 32 * 32)));
	map.parameters().centroid_interval_length_rad = M_PI;
	map.parameters().order_type = cartocrow::necklace_map::OrderType::kAny;
	map.parameters().heuristic_cycles = 0;
	map.parameters().placement_cycles = 10;

	SECTION("unit-sized beads") {
		map.addBead("R1", 1, necklace);
		map.addBead("R2", 1, necklace);
		map.compute();
		CHECK(map.scaleFactor() == Approx(32.0).epsilon(0.01));
	}
	SECTION("larger sized beads") {
		map.addBead("R1", 2, necklace);
		map.addBead("R2", 2, necklace);
		map.compute();
		renderer::IpeRenderer r((Painting(map)));
		r.save("/tmp/bla.ipe");

		CHECK(map.scaleFactor() == Approx(32.0 / std::sqrt(2)).epsilon(0.01));
	}
}
