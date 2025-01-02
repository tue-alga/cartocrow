#ifndef CARTOCROW_INPUT_PARSING_H
#define CARTOCROW_INPUT_PARSING_H

#include <gdal/ogrsf_frmts.h>

#include <filesystem>

#include "../core/region_map.h"

namespace cartocrow::chorematic_map {
using RegionWeight = std::unordered_map<std::string, double>;

std::shared_ptr<std::unordered_map<std::string, RegionWeight>>
regionDataMapFromGPKG(const std::filesystem::path &path, const std::string &layerName,
                      const std::string &regionNameAttribute,
                      const std::function<std::string(std::string)>& regionNameTransform);

std::shared_ptr<RegionMap> regionMapFromGPKG(const std::filesystem::path &path,
                                             const std::string &layerName,
                                             const std::string &regionNameAttribute,
                                             const std::optional<std::function<bool(
                                                     const OGRFeature&)>> &skip = std::nullopt);
}
#endif //CARTOCROW_INPUT_PARSING_H
