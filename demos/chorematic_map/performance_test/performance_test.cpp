#include "cartocrow/core/region_map.h"
#include "cartocrow/core/region_arrangement.h"

#include "cartocrow/chorematic_map/parse_points.h"
#include "cartocrow/chorematic_map/maximum_weight_disk.h"
#include "cartocrow/chorematic_map/choropleth_disks.h"
#include "cartocrow/chorematic_map/sampler.h"
#include "cartocrow/chorematic_map/input_parsing.h"

#include <chrono>

#include <CGAL/point_generators_2.h>

#include <gdal/ogrsf_frmts.h>

using namespace cartocrow;
using namespace cartocrow::chorematic_map;

void measureDiskRunningTime() {
	int nRuns = 1;

	std::vector<std::pair<int, double>> times;

	for (int n = 2; n <= 1000; n += 10) {
		std::cout << n << std::endl;
		double totalTime = 0;
		for (int run = 0; run < nRuns; ++run) {
			std::cout << "\t" << run << std::endl;
			int seed = 0;
			CGAL::Random rng(seed);
			CGAL::get_default_random() = CGAL::Random(seed);
			auto generator = CGAL::Random_points_in_square_2<Point<Exact>>(1, rng);

			std::vector<Point<Exact>> points;
			std::copy_n(generator, n, std::back_inserter(points));
			std::vector<WeightedPoint> weightedPoints;
			for (int i = 0; i < n / 2; ++i) {
				weightedPoints.emplace_back(approximate(points[i]), 1);
				weightedPoints.emplace_back(approximate(points[n / 2 + i]), -1);
			}
			auto begin = std::chrono::steady_clock::now();
			auto disk = smallest_maximum_weight_disk(weightedPoints.begin(), weightedPoints.end());
			auto end = std::chrono::steady_clock::now();
			totalTime += std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000.0;
		}

		times.emplace_back(n, totalTime / nRuns);
	}

	for (const auto& [n, time] : times) {
		std::cout << "(" << n << ", " << time << ") ";
	}
}

void measureSamplingRunningTime() {
	int seed = 0;

	std::filesystem::path dutch = "data/chorematic_map/gemeenten-2022_19282vtcs.ipe";
	auto regionMap = std::make_shared<RegionMap>(ipeToRegionMap(dutch, true));
	auto regionArr = std::make_shared<RegionArrangement>(regionMapToArrangementParallel(*regionMap));
	std::vector<std::pair<std::string, std::function<
	                                       std::function<WeightedRegionSample<Exact>()>
	                                       (Sampler<Landmarks_pl<RegionArrangement>>&, int n)
	                                       >>> methods = {
	    {
	        "uniform",
	        [&regionArr](Sampler<Landmarks_pl<RegionArrangement>>& sampler, int n) {
		        return [&sampler, n]() { return sampler.uniformRandomSamples(n); };
	        }
	    },
	    {
	        "square_grid",
	        [&regionArr](Sampler<Landmarks_pl<RegionArrangement>>& sampler, int n) {
		        auto [cellSize, _] = sampler.squareGrid(n);
		        return [&sampler, &cellSize]() {
					return sampler.squareGrid(cellSize);
		        };
	        }
	    },
	    {
	        "hex_grid",
	        [&regionArr](Sampler<Landmarks_pl<RegionArrangement>>& sampler, int n) {
			  auto [cellSize, _] = sampler.hexGrid(n);
			  return [&sampler, &cellSize]() {
				return sampler.hexGrid(cellSize);
			  };
	        }
	    },
	    {
	        "Voronoi_1",
			[&regionArr](Sampler<Landmarks_pl<RegionArrangement>>& sampler, int n) {
			    return [&sampler, n]() { return sampler.voronoiUniform(n, 1); };
	        }
	    },
	    {
	        "Voronoi_5",
			[&regionArr](Sampler<Landmarks_pl<RegionArrangement>>& sampler, int n) {
			  return [&sampler, n]() { return sampler.voronoiUniform(n, 5); };
	        }
	    },
	    {
	        "Voronoi_25",
			[&regionArr](Sampler<Landmarks_pl<RegionArrangement>>& sampler, int n) {
			  	return [&sampler, n]() { return sampler.voronoiUniform(n, 25); };
	        }
	    },
	    {
	        "Voronoi_100",
	        [&regionArr](Sampler<Landmarks_pl<RegionArrangement>>& sampler, int n) {
			 return [&sampler, n]() { return sampler.voronoiUniform(n, 100); };
		 	}
	    }
	};

	for (const auto& [name, method] : methods) {
		for (bool samplePerRegion : {false, true}) {
			Sampler<Landmarks_pl<RegionArrangement>> sampler(regionArr, seed, samplePerRegion);
			// todo: determine what preprocessing should be measured and what not
			if (samplePerRegion) {
				sampler.computeRegionCCs();
			} else {
				sampler.computeLandmasses();
			}
			sampler.getArrBoundingBox();
			sampler.getPL();
			auto f = method(sampler, 1000);
			auto begin = std::chrono::steady_clock::now();
			f();
			auto end = std::chrono::steady_clock::now();
			auto secs = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() /
			            1000000.0;
			std::cout << name << (samplePerRegion ? "_perRegion" : "") << ": " << secs << std::endl;
		}
	}
}

void measureScores() {
	std::filesystem::path dutch = "data/chorematic_map/gemeenten-2022_19282vtcs.ipe";
	auto regionMap = std::make_shared<RegionMap>(ipeToRegionMap(dutch, true));
	auto regionArr = std::make_shared<RegionArrangement>(regionMapToArrangementParallel(*regionMap));
	std::vector<std::pair<std::string, std::function<
	                                       std::function<WeightedRegionSample<Exact>()>
	                                       (Sampler<Landmarks_pl<RegionArrangement>>&, int n)
	                                       >>> methods = {
//	        {
//	            "uniform",
//	            [&regionArr](Sampler<Landmarks_pl<RegionArrangement>>& sampler, int n) {
//		            return [&sampler, n]() { return sampler.uniformRandomSamples(n); };
//	            }
//	        },
//	        {
//	            "square_grid",
//	            [&regionArr](Sampler<Landmarks_pl<RegionArrangement>>& sampler, int n) {
//		            auto [cellSize, _] = sampler.squareGrid(n);
//		            return [&sampler, &cellSize]() {
//			            return sampler.squareGrid(cellSize);
//		            };
//	            }
//	        },
//	        {
//	            "hex_grid",
//	            [&regionArr](Sampler<Landmarks_pl<RegionArrangement>>& sampler, int n) {
//		            auto [cellSize, _] = sampler.hexGrid(n);
//		            return [&sampler, &cellSize]() {
//			            return sampler.hexGrid(cellSize);
//		            };
//	            }
//	        },
	        {
	            "Voronoi_1",
	            [&regionArr](Sampler<Landmarks_pl<RegionArrangement>>& sampler, int n) {
		            return [&sampler, n]() { return sampler.voronoiUniform(n, 1); };
	            }
	        },
//	        {
//	            "Voronoi_5",
//	            [&regionArr](Sampler<Landmarks_pl<RegionArrangement>>& sampler, int n) {
//		            return [&sampler, n]() { return sampler.voronoiUniform(n, 5); };
//	            }
//	        },
//	        {
//	            "Voronoi_25",
//	            [&regionArr](Sampler<Landmarks_pl<RegionArrangement>>& sampler, int n) {
//		            return [&sampler, n]() { return sampler.voronoiUniform(n, 25); };
//	            }
//	        },
//	        {
//	            "Voronoi_100",
//	            [&regionArr](Sampler<Landmarks_pl<RegionArrangement>>& sampler, int n) {
//		            return [&sampler, n]() { return sampler.voronoiUniform(n, 100); };
//	            }
//	        }
	    };

	std::filesystem::path gpkg3 = "data/chorematic_map/wijkenbuurten_2022_v3.gpkg";
	auto regionWeightMap = regionDataMapFromGPKG(gpkg3, "gemeenten", "gemeentecode", [](const std::string& s) {
		return s;
	});
	auto regionWeight = std::make_shared<RegionWeight>((*regionWeightMap)["percentage_huishoudens_met_hoog_inkomen"]);
	Choropleth choropleth(regionArr, regionWeight, 2);

	int nSeeds = 1;

	std::ofstream fileOut("scores-with-heuristic.txt");
	for (bool samplePerRegion : {true}) {
		Sampler<Landmarks_pl<RegionArrangement>> sampler(regionArr, 0, samplePerRegion);

		for (const auto& [name, method] : methods) {
			fileOut << name << (samplePerRegion ? "_perRegion" : "") << std::endl;
			for (int n = 10; n <= 1000; n += 10) {
				double totalScore = 0;
				for (int seed = 0; seed < nSeeds; ++seed) {
					sampler.setSeed(seed);
					auto f = method(sampler, n);
					auto sample = f();
					std::vector<WeightedPoint> weightedPoints;
					sample.weightedPoints(std::back_inserter(weightedPoints), *regionWeight);
					auto disk = fitDisks(choropleth, sample, false, true, true)[0];
					totalScore += disk.score.value();
				}
				fileOut << "(" << n << ", " << (totalScore / nSeeds) << ")" << std::endl;
			}
		}
	}
}

int main() {
//	measureSamplingRunningTime();
	measureScores();
}