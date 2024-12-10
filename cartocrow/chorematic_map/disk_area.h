#ifndef CARTOCROW_DISK_AREA_H
#define CARTOCROW_DISK_AREA_H

#include "weighted_point.h"
#include "../core/region_arrangement.h"

namespace cartocrow::chorematic_map {
Number<Inexact> totalWeight(const Circle<Inexact>& disk, std::shared_ptr<RegionArrangement> arr,
                          const std::unordered_map<std::string, double>& regionWeights);
}

#endif //CARTOCROW_DISK_AREA_H
