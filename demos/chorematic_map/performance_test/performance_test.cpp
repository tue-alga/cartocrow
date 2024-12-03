#include "cartocrow/chorematic_map/parse_points.h"
#include "cartocrow/chorematic_map/maximum_weight_disk.h"
#include <chrono>

using namespace cartocrow;
using namespace cartocrow::chorematic_map;

int main() {
	std::vector<WeightedPoint> points = readPointsFromIpe("data/chorematic_map/performance.ipe")[0];
	auto begin = std::chrono::steady_clock::now();
	auto disk = smallest_maximum_weight_disk(points.begin(), points.end());
	auto end = std::chrono::steady_clock::now();
	std::cout << "Duration (sec) = " <<  (std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()) /1000000.0  <<std::endl;
}