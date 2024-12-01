#include "../catch.hpp"

#include "cartocrow/chorematic_map/maximum_weight_disk.h"
#include "cartocrow/chorematic_map/parse_points.h"

namespace cartocrow::chorematic_map {
TEST_CASE("Maximum weight disk") {
	auto pointSets = readPointsFromIpe("data/chorematic_map/maximum_weight_disk_tests.ipe");
	auto disks = readDisksFromIpe("data/chorematic_map/maximum_weight_disk_tests.ipe");
	if (pointSets.size() != disks.size()) {
		throw std::runtime_error("Impossible: there should be a one point set and one disk per page in the ipe file.");
	}
	for (int i = 0; i < pointSets.size(); ++i) {
		auto& points = pointSets[i];
		auto [p1, p2, p3] = disks[i];
		auto [wp1, wp2, wp3] = maximum_weight_disk(points.begin(), points.end());
		if (!wp1.has_value()) {
			CHECK(!p1.has_value());
			continue;
		}
		CHECK(p1.has_value());
		if (!wp2.has_value()) {
			CHECK(!p2.has_value());
			CHECK(wp1->point == p1);
			continue;
		}
		CHECK(p2.has_value());
		if (!wp3.has_value()) {
			CHECK(!p3.has_value());
			CHECK((wp1->point == p1 && wp2->point == p2 || wp1->point == p2 && wp2->point == p1));
			continue;
		}
		CHECK(p3.has_value());
		std::vector<Point<Inexact>> ps({*p1, *p2, *p3});
		std::vector<Point<Inexact>> qs({wp1->point, wp2->point, wp3->point});
		std::sort(ps.begin(), ps.end());
		std::sort(qs.begin(), qs.end());
		CHECK(ps == qs);
	}
}
}