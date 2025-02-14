#include "cartocrow/core/region_map.h"
#include "cartocrow/core/region_arrangement.h"

#include "cartocrow/chorematic_map/parse_points.h"
#include "cartocrow/chorematic_map/maximum_weight_disk.h"
#include "cartocrow/chorematic_map/choropleth_disks.h"
#include "cartocrow/chorematic_map/sampler.h"
#include "cartocrow/chorematic_map/input_parsing.h"

#include "cartocrow/renderer/ipe_renderer.h"

#include <chrono>

#include <CGAL/point_generators_2.h>

using namespace cartocrow;
using namespace cartocrow::renderer;
using namespace cartocrow::chorematic_map;

void measureDiskRunningTime() {
	int nRuns = 1;

	std::vector<std::pair<int, double>> times;

	for (int n = 1000; n <= 1000; n += 10) {
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

	std::filesystem::path dutch = "data/chorematic_map/gemeenten-2022_5000vtcs.ipe";
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
//		        auto [cellSize, _] = sampler.squareGrid(n);
		        return [&sampler, n]() {
					return sampler.squareGrid(n);
		        };
	        }
	    },
	    {
	        "hex_grid",
	        [&regionArr](Sampler<Landmarks_pl<RegionArrangement>>& sampler, int n) {
//			  auto [cellSize, _] = sampler.hexGrid(n);
			  return [&sampler, n]() {
				return sampler.hexGrid(n);
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
	bool heuristic = false;

	std::filesystem::path dutch = "data/chorematic_map/gemeenten-2022_5000vtcs.ipe";
	auto regionMapDutch = std::make_shared<RegionMap>(ipeToRegionMap(dutch, true));
	auto regionArrDutch = std::make_shared<RegionArrangement>(regionMapToArrangementParallel(*regionMapDutch));

	std::filesystem::path gpkgDutch = "data/chorematic_map/wijkenbuurten_2022_v3.gpkg";
	auto regionWeightMapDutch = regionDataMapFromGPKG(gpkgDutch, "gemeenten", "gemeentecode", [](const std::string& s) {
	  return s;
	});

	std::filesystem::path gpkgHessen = "data/chorematic_map/hessen.gpkg";
	auto regionMapHessen = regionMapFromGPKG(gpkgHessen, "Hessen", "GEN");
	auto regionArrHessen = std::make_shared<RegionArrangement>(regionMapToArrangementParallel(*regionMapHessen));
	auto regionWeightMapHessen = regionDataMapFromGPKG(gpkgHessen, "Hessen", "GEN", [](const std::string& s) {
		return s;
	});

	std::vector<std::tuple<std::shared_ptr<RegionArrangement>, std::string, std::shared_ptr<std::unordered_map<std::string, RegionWeight>>, std::string>>
	     choropleths({
						{regionArrHessen, "hessen", regionWeightMapHessen, "mun_stats_water_normalized"},
						{regionArrHessen, "hessen", regionWeightMapHessen, "mun_stats_vegetation_normalized"},
						{regionArrHessen, "hessen", regionWeightMapHessen, "mun_stats_vege_forest_normalized"},
						{regionArrHessen, "hessen", regionWeightMapHessen, "mun_stats_veg_agrar_nomalized"},
						{regionArrHessen, "hessen", regionWeightMapHessen, "mun_stats_sie_wohnbau_normalized"},
						{regionArrHessen, "hessen", regionWeightMapHessen, "mun_stats_total_income_1k_normalized"},
						{regionArrDutch, "dutch", regionWeightMapDutch, "apotheek_gemiddelde_afstand_in_km"},
						{regionArrDutch, "dutch", regionWeightMapDutch, "brandweerkazerne_gemiddelde_afstand_in_km"},
						{regionArrDutch, "dutch", regionWeightMapDutch, "kunstijsbaan_gemiddelde_afstand_in_km"},
						{regionArrDutch, "dutch", regionWeightMapDutch, "percentage_huishoudens_met_hoog_inkomen"},
						{regionArrDutch, "dutch", regionWeightMapDutch, "percentage_huurwoningen"},
						{regionArrDutch, "dutch", regionWeightMapDutch, "percentage_werknemers"}
					});

	std::vector<std::pair<std::string, std::function<
	                                       std::function<WeightedRegionSample<Exact>()>
	                                       (Sampler<Landmarks_pl<RegionArrangement>>&, int n)
	                                       >>> methods = {
	        {
	            "Random",
	            [](Sampler<Landmarks_pl<RegionArrangement>>& sampler, int n) {
		            return [&sampler, n]() { return sampler.uniformRandomSamples(n); };
	            }
	        },
	        {
	            "Square",
	            [](Sampler<Landmarks_pl<RegionArrangement>>& sampler, int n) {
		            return [&sampler, n]() {
			            return sampler.squareGrid(n);
		            };
	            }
	        },
	        {
	            "Hex",
	            [](Sampler<Landmarks_pl<RegionArrangement>>& sampler, int n) {
		            return [&sampler, n]() {
			            return sampler.hexGrid(n);
		            };
	            }
	        },
	        {
	            "Voronoi",
	            [](Sampler<Landmarks_pl<RegionArrangement>>& sampler, int n) {
		            return [&sampler, n]() { return sampler.voronoiUniform(n, 25); };
	            }
	        },
	    };

	int delta = 5;
	std::vector<int> marks;
	for (int mark = 100; mark <= 1000; mark += 100) {
		marks.push_back(mark);
	}
//	marks.push_back(10000);
	int seed = 0;
	for (bool samplePerRegion : {true, false}) {
		std::stringstream outputFileName;
		outputFileName << "scores" << (samplePerRegion ? "-local" : "-global") << ".txt";
		std::ofstream fileOut(outputFileName.str());
		fileOut << std::setprecision(16);

		for (const auto& [regionArr, mapName, regionWeightMap, attribute] : choropleths) {
			Sampler<Landmarks_pl<RegionArrangement>> sampler(regionArr, 0, samplePerRegion);

			auto regionWeight = std::make_shared<RegionWeight>((*regionWeightMap)[attribute]);
			Choropleth choropleth(regionArr, regionWeight, 2);

			for (bool invert : {false, true}) {
				for (const auto& [methodName, method] : methods) {
					for (int mark : marks) {
						for (int n = mark - delta; n <= mark + delta; ++n) {
							sampler.setSeed(seed);
							auto f = method(sampler, n);
							auto sample = f();
							++seed;
							if (sample.m_points.size() != n) {
								std::cerr << "Incorrect number of samples!" << std::endl;
								std::cerr << mapName << " " << attribute << " " << methodName << " "
								          << n << std::endl;
								continue;
							}
							std::vector<WeightedPoint> weightedPoints;
							sample.weightedPoints(std::back_inserter(weightedPoints), *regionWeight);
							auto disk = fitDisks(choropleth, sample, invert, true, heuristic)[0];
							fileOut << mapName << "," << attribute << "," << invert << "," << methodName << ","
							        << (sample.m_points.size()) << "," << disk.score.value() << ","
							        << seed << std::endl;
						}
					}
				}
			}
		}
	}
}

int main() {
//	measureDiskRunningTime();
	measureScores();
}