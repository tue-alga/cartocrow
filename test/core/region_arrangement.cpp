#include "../catch.hpp"

#include <filesystem>

#include "../../cartocrow/core/region_arrangement.h"

using namespace cartocrow;

TEST_CASE("Converting a region map to an arrangement") {
	RegionMap map = ipeToRegionMap(std::filesystem::path("data/test_region_map.ipe"));
	CHECK(map.size() == 2);
	CHECK(map.contains("R1"));
	CHECK(map.contains("R2"));

	RegionArrangement<> arrangement = regionMapToArrangement(map);
	CHECK(arrangement.number_of_faces() == 4); // R1, R2 (2 pieces), outer face
	int num_r1 = 0;
	int num_r2 = 0;
	int num_no_id = 0;
	for (auto face_iterator = arrangement.faces_begin(); face_iterator != arrangement.faces_end();
	     ++face_iterator) {
		if (face_iterator->data().name == "R1") {
			num_r1++;
		} else if (face_iterator->data().name == "R2") {
			num_r2++;
		} else if (face_iterator->data().name == "") {
			num_no_id++;
		} else {
			FAIL_CHECK();
		}
	}
	CHECK(num_r1 == 1);
	CHECK(num_r2 == 2);
	CHECK(num_no_id == 1);
}

TEST_CASE("Converting overlapping regions to an arrangement (should throw)") {
	RegionMap map = ipeToRegionMap(std::filesystem::path("data/test_region_map_overlap.ipe"));
	CHECK(map.size() == 2);
	CHECK(map.contains("R1"));
	CHECK(map.contains("R2"));
	CHECK_THROWS_WITH(regionMapToArrangement(map), Catch::StartsWith("Found overlapping regions "));
}
