#include "../catch.hpp"

#include "../../cartocrow/core/region_map.h"
#include "../../cartocrow/necklace_map/circle_necklace.h"
#include "../../cartocrow/necklace_map/necklace_map.h"

using namespace cartocrow;
using namespace cartocrow::necklace_map;

TEST_CASE("Constructing a necklace map and running the computation") {
	std::shared_ptr<RegionMap> regions = std::make_shared<RegionMap>(
	    ipeToRegionMap(std::filesystem::path("data/test_region_map.ipe")));
	NecklaceMap map(regions);
	auto necklace = map.addNecklace(
	    std::make_unique<CircleNecklace>(Circle<Inexact>(Point<Inexact>(64, 32), 32 * 32)));
	map.addBead("R1", 1, necklace);
	map.addBead("R2", 1, necklace);
	map.compute();
	CHECK(map.scaleFactor() == Approx(32.0));
}
