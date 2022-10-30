#include "../catch.hpp"

#include "../../cartocrow/core/core.h"
#include "../../cartocrow/core/region_map.h"
#include "../../cartocrow/core/region_arrangement.h"
#include "../../cartocrow/simplification/visvalingam_whyatt/vw.h"

using namespace cartocrow;
using namespace cartocrow::simplification;

TEST_CASE("Simplifying a simple map by one") {
	
	/*std::shared_ptr<RegionMap> regions = std::make_shared<RegionMap>(
	    ipeToRegionMap(std::filesystem::path("data/???.ipe")));
	
	auto map = regionMapToArrangement<VWVertex>(*regions);

	REQUIRE(map.number_of_vertices() == 10);

	VWSimplification vw(map);
	vw.simplify(0, 5000000);

	REQUIRE(map.number_of_vertices() == 9);*/
}
