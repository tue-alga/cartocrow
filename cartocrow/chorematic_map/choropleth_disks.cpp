#include "choropleth_disks.h"
#include "maximum_weight_disk.h"
#include "disk_area.h"
#include "../core/cs_polygon_helpers.h"

#include <CGAL/Boolean_set_operations_2.h>

namespace cartocrow::chorematic_map {
/// The disks are returned in the order that they should be drawn.
std::vector<BinDisk> fitDisks(const Choropleth& choropleth, const WeightedRegionSample<Exact>& sample,
                              bool invert, bool computeScores, bool heuristic) {
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
		for (const auto& [region, _] : *(choropleth.m_data)) {
			auto bin = choropleth.regionToBin(region);
			if (!bin.has_value()) continue;

			regionWeight[region] = CGAL::to_double(*bin == binToFit ? 1 : -positiveArea/negativeArea);
		}

		std::vector<WeightedPoint> weightedPoints;
		sample.weightedPoints(std::back_inserter(weightedPoints), regionWeight);

		if (!binDisks.empty()) {
			auto it = std::remove_if(
			    weightedPoints.begin(), weightedPoints.end(), [&binDisks](const WeightedPoint& wp) {
				    if (wp.weight > 0) return false;
				    for (const auto& disk : binDisks) {
					    std::optional<Circle<Exact>> circle = disk.disk;
					    if (circle.has_value() && !circle->has_on_unbounded_side(makeExact(wp.point))) {
						    return true;
					    }
				    }
				    return false;
			    });
			weightedPoints.erase(it, weightedPoints.end());
		}

		std::optional<Circle<Exact>> circle;
		InducedDiskW iDisk = smallest_maximum_weight_disk(weightedPoints.begin(), weightedPoints.end());
		auto [p1, p2, p3] = iDisk;
		if (p1.has_value() && p2.has_value() && p3.has_value()) {
			circle = Circle<Exact>(makeExact(p1->point), makeExact(p2->point), makeExact(p3->point));
		} else if (p1.has_value() && p2.has_value()) {
			circle = Circle<Exact>(makeExact(p1->point), makeExact(p2->point));
		} else {
			circle = std::nullopt;
		}
		binDisks.emplace_back(binToFit, circle, std::nullopt);

		if (computeScores) {
			if (binsToFit.size() > 1) {
				std::cerr << "Score computation of multiple disks has not been implemented." << std::endl;
			}

			if (!circle.has_value()) {
				binDisks.back().score = 0;
				continue;
			}

			auto& arr = choropleth.m_arr;

			binDisks.back().score = totalWeight(*circle, *arr, regionWeight);
		}
	}

	std::reverse(binDisks.begin(), binDisks.end());
	return binDisks;
}

std::pair<Circle<Inexact>, double>
perturbDiskRadius(const Circle<Inexact>& disk,
                  double score,
                  const RegionArrangement& arr,
                  const std::unordered_map<std::string, double>& regionWeight,
                  double maxDeltaRadius,
                  int iterations) {
	double bestScore = score;
	Circle<Exact> bestDisk = makeExact(disk);
	for (int i = 0; i < 10; ++i) {
		Circle<Exact> diskP(bestDisk.center(), sqrt(CGAL::to_double(disk.squared_radius())) + i * maxDeltaRadius / iterations);
		auto scoreP = totalWeight(diskP, arr, regionWeight);
		if (scoreP > bestScore) {
			bestDisk = diskP;
			bestScore = scoreP;
		}
	}
	return {approximate(bestDisk), bestScore};
}
}