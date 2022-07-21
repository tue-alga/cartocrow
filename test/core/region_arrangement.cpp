#include "../catch.hpp"

#include <filesystem>

#include "../../cartocrow/core/region_arrangement.h"

using namespace cartocrow;

TEST_CASE("Converting a region map to an arrangement") {
	RegionMap map = ipeToRegionMap(std::filesystem::path("data/test_region_map.ipe"));
	CHECK(map.size() == 2);
	CHECK(map.contains("R1"));
	CHECK(map.contains("R2"));

	RegionArrangement arrangement = regionMapToArrangement(map);
	CHECK(arrangement.number_of_faces() == 4); // R1, R2 (2 pieces), outer face
}
