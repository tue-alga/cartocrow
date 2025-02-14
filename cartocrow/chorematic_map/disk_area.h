#ifndef CARTOCROW_DISK_AREA_H
#define CARTOCROW_DISK_AREA_H

#include "weighted_point.h"
#include "general_circle.h"
#include "../core/region_arrangement.h"

namespace cartocrow::chorematic_map {
Number<Inexact> totalWeight(const Circle<Exact>& disk, const RegionArrangement& arr,
                            const std::unordered_map<std::string, double>& regionWeights);
Number<Inexact> totalWeight(const GeneralCircle<Exact>& gDisk, const RegionArrangement& arr,
							const std::unordered_map<std::string, double>& regionWeights);
}

#endif //CARTOCROW_DISK_AREA_H
