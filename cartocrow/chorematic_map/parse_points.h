#ifndef CARTOCROW_PARSE_POINTS_H
#define CARTOCROW_PARSE_POINTS_H

#include "weighted_point.h"
#include "cartocrow/reader/ipe_reader.h"

namespace cartocrow::chorematic_map {
std::vector<WeightedPoint> readPointsFromIpePage(ipe::Page* page, ipe::Cascade* cascade);
std::vector<std::vector<WeightedPoint>> readPointsFromIpe(const std::filesystem::path& path);
std::vector<InducedDisk> readDisksFromIpe(const std::filesystem::path& path);
InducedDisk readDiskFromIpePage(ipe::Page* page);
}
#endif //CARTOCROW_PARSE_POINTS_H
