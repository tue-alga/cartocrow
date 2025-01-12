#ifndef CARTOCROW_CHOROPLETH_DISKS_H
#define CARTOCROW_CHOROPLETH_DISKS_H

#include "choropleth.h"
#include "general_circle.h"
#include "weighted_region_sample.h"

namespace cartocrow::chorematic_map {
struct BinDisk {
	int bin;
	std::optional<GeneralCircle<Exact>> disk;
	std::optional<double> score;
};

std::vector<BinDisk> fitDisks(const Choropleth& choropleth, const WeightedRegionSample<Exact>& sample,
                              bool invert = false, bool computeScores = false, bool heuristic = false,
                              bool symmetricDifference = false);

std::pair<GeneralCircle<Exact>, double>
perturbDiskRadius(const GeneralCircle<Exact>& disk,
				  double score,
				  const RegionArrangement& arr,
				  const std::unordered_map<std::string, double>& regionWeight,
				  double maxDeltaRadius,
				  int iterations);
}

#endif //CARTOCROW_CHOROPLETH_DISKS_H
