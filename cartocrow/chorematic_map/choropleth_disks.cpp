#include "choropleth_disks.h"
#include "cartocrow/circle_segment_helpers/cs_polygon_helpers.h"
#include "disk_area.h"
#include "maximum_weight_disk.h"

namespace cartocrow::chorematic_map {
std::vector<BinDisk> fitDisks(const Choropleth& choropleth, const WeightedRegionSample<Exact>& sample,
                              bool invert, bool computeScores, bool heuristic, bool symmetricDifference) {
	using RegionWeight = WeightedRegionSample<Exact>::RegionWeight;

	std::vector<int> binsToFit;
	if (invert) {
		for (int binToFit = 0; binToFit < choropleth.numberOfBins() - 1; ++binToFit) {
			binsToFit.push_back(binToFit);
		}
	} else {
		for (int binToFit = choropleth.numberOfBins() - 1; binToFit >= 1; --binToFit) {
			binsToFit.push_back(binToFit);
		}
	}

	std::vector<BinDisk> binDisks;
	for (int binToFit : binsToFit) {
		// Compute weights for this bin
		RegionWeight regionWeight;
		Number<Exact> negativeArea = 0;
		auto binAreas = choropleth.binAreas();
		for (int i = 0; i < binAreas.size(); ++i) {
			if (i == binToFit) continue;
			auto& binArea = binAreas[i];
			negativeArea += binArea;
		}
		auto& positiveArea = binAreas[binToFit];
		auto totalArea = negativeArea + positiveArea;
		for (const auto& [region, _] : *(choropleth.m_data)) {
			auto bin = choropleth.regionToBin(region);
			if (!bin.has_value()) continue;

            if (!symmetricDifference) {
                regionWeight[region] = CGAL::to_double(*bin == binToFit ? negativeArea / totalArea : -positiveArea / totalArea);
            } else {
                regionWeight[region] = CGAL::to_double(*bin == binToFit ? 1 : -1);
            }
		}

		std::vector<WeightedPoint> weightedPoints;
		sample.weightedPoints(std::back_inserter(weightedPoints), regionWeight);

		if (!binDisks.empty()) {
			auto it = std::remove_if(
			    weightedPoints.begin(), weightedPoints.end(), [&binDisks](const WeightedPoint& wp) {
				    if (wp.weight > 0) return false;
				    for (const auto& disk : binDisks) {
					    auto circle = disk.disk;
					    if (circle.has_value() && !circle->has_on_unbounded_side(pretendExact(wp.point))) {
						    return true;
					    }
				    }
				    return false;
			    });
			weightedPoints.erase(it, weightedPoints.end());
		}

		std::optional<GeneralCircle<Exact>> circle;
		InducedDiskW iDisk = smallest_maximum_weight_disk(weightedPoints.begin(), weightedPoints.end());
		auto [p1, p2, p3] = iDisk;
		if (p1.has_value() && p2.has_value() && p3.has_value()) {
			if (abs(Triangle<Inexact>(p1->point, p2->point, p3->point).area()) < M_EPSILON) {
				circle = Halfplane<Exact>(Line<Exact>(pretendExact(p1->point), pretendExact(p2->point)));
			} else {
				circle =
				    Circle<Exact>(pretendExact(p1->point), pretendExact(p2->point),
				                       pretendExact(p3->point));
			}
		} else if (p1.has_value() && p2.has_value()) {
			circle = Circle<Exact>(pretendExact(p1->point), pretendExact(p2->point));
		} else {
			circle = std::nullopt;
		}
		binDisks.emplace_back(binToFit, circle, std::nullopt);

		if (computeScores || heuristic) {
			if (binsToFit.size() > 1) {
				std::cerr << "Score computation of multiple disks has not been implemented." << std::endl;
			}

			if (!circle.has_value()) {
				binDisks.back().score = 0;
				continue;
			}

			auto& arr = choropleth.m_arr;

			double normalizer = CGAL::to_double((positiveArea * negativeArea) / totalArea);
			binDisks.back().score = totalWeight(*circle, *arr, regionWeight) / normalizer;

            if (heuristic) {
                double areaPerPoint = (CGAL::to_double(positiveArea) + CGAL::to_double(negativeArea)) / sample.m_points.size();
                double deltaRadius = sqrt(areaPerPoint) * 2;
                auto [bDisk, bScore] = perturbDiskRadius(binDisks.back().disk.value(), binDisks.back().score.value(),
                                                         *arr, regionWeight, deltaRadius, 20, normalizer);
                binDisks.back().disk = bDisk;
                binDisks.back().score = bScore;
            }
		}
	}

	std::reverse(binDisks.begin(), binDisks.end());
	return binDisks;
}

std::pair<GeneralCircle<Exact>, double>
perturbDiskRadius(const GeneralCircle<Exact>& generalDisk,
                  double score,
                  const RegionArrangement& arr,
                  const std::unordered_map<std::string, double>& regionWeight,
                  double maxDeltaRadius,
                  int iterations,
                  double normalizer) {
	if (generalDisk.is_halfplane()) {
		return {generalDisk, score};
	}
	auto disk = generalDisk.get_circle();
	double bestScore = score;
	Circle<Exact> bestDisk = disk;
	for (int i = 1; i <= iterations; ++i) {
        double r = sqrt(CGAL::to_double(disk.squared_radius())) + i * maxDeltaRadius / iterations;
		Circle<Exact> diskP(bestDisk.center(), r * r);
		auto scoreP = totalWeight(diskP, arr, regionWeight) / normalizer;
		if (scoreP > bestScore) {
			bestDisk = diskP;
			bestScore = scoreP;
		}
	}
	return {bestDisk, bestScore};
}
}