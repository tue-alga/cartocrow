#ifndef CARTOCROW_CHOROPLETH_DISKS_H
#define CARTOCROW_CHOROPLETH_DISKS_H

#include "choropleth.h"
#include "weighted_region_sample.h"

namespace cartocrow::chorematic_map {
struct BinDisk {
	int bin;
	InducedDiskW disk;
	std::optional<double> score;
};
std::vector<BinDisk> fitDisks(const Choropleth& choropleth, const WeightedRegionSample<Exact>& sample,
                              bool invert = false, bool computeScores = false);
}

#endif //CARTOCROW_CHOROPLETH_DISKS_H
