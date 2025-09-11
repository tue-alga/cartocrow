#ifndef CARTOCROW_INPUT_PARSING_H
#define CARTOCROW_INPUT_PARSING_H

#include <ogrsf_frmts.h>

#include <filesystem>

#include "../core/region_map.h"

namespace cartocrow::chorematic_map {
using RegionWeight = std::unordered_map<std::string, double>;

std::shared_ptr<std::unordered_map<std::string, RegionWeight>>
regionDataMapFromGPKG(const std::filesystem::path& path,
					  const std::string& regionNameAttribute,
                      const std::optional<std::string>& layerName = std::nullopt,
                      const std::function<std::string(std::string)>& regionNameTransform = [](const std::string& s) { return s; });

std::shared_ptr<RegionMap> regionMapFromGPKG(const std::filesystem::path &path,
                                             const std::string &regionNameAttribute,
											 const std::optional<std::string>& layerName = std::nullopt,
                                             const std::optional<std::function<bool(
                                                     const OGRFeature&)>> &skip = std::nullopt);

std::unordered_map<std::string, double> parseRegionData(const std::string& s, char delimiter = ',');

std::string regionDataToCSV(const std::unordered_map<std::string, double>& regionData);
}
#endif //CARTOCROW_INPUT_PARSING_H
