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
	BinDisk() = default;
	BinDisk(int bin, std::optional<GeneralCircle<Exact>> disk = std::nullopt, std::optional<double> score = std::nullopt) :
		bin(bin), disk(disk), score(score) {};
};

/// This function takes a \ref Choropleth and a point sample (\ref Sampler implements sampling methods),
/// and returns disks fit to a class of the choropleth.
/// The function is setup to be general, but currently only properly supports choropleths of two classes.
/// By default a disk is fit to the second class of the choropleth (the one with higher values), but the
/// \p invert parameter can be set to true to fit to the first class instead.
/// When the \p computeScores parameter is set, then the score field of the \ref BinDisk is set to its normalized score.
/// When the \p heuristic parameter is set, the radius of the disk is perturbed to locally optimize the score of the disk.
std::vector<BinDisk> fitDisks(const Choropleth& choropleth, const WeightedRegionSample<Exact>& sample,
                              bool invert = false, bool computeScores = false, bool heuristic = false,
                              bool symmetricDifference = false);

std::pair<GeneralCircle<Exact>, double>
perturbDiskRadius(const GeneralCircle<Exact>& disk,
				  double score,
				  const RegionArrangement& arr,
				  const std::unordered_map<std::string, double>& regionWeight,
				  double maxDeltaRadius,
				  int iterations,
                  double normalizer);
}

#endif //CARTOCROW_CHOROPLETH_DISKS_H
