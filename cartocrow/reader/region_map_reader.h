#ifndef CARTOCROW_REGION_MAP_READER_H
#define CARTOCROW_REGION_MAP_READER_H

#include "cartocrow/core/region_map.h"

namespace cartocrow {
/// Creates a \ref RegionMap from a region map in Ipe format.
///
/// The Ipe figure to be read needs to contain a single page. This page
/// has polygonal shapes (possibly containing holes or separate connected
/// components), each representing a region. Each region then needs to contain
/// exactly one label in its interior, indicating the name of the region.
///
/// Throws if the file could not be read, if the file is not a valid Ipe file,
/// or if the file does not contain regions like specified above.
RegionMap ipeToRegionMap(const std::filesystem::path& file, bool labelAtCentroid = false);
}

#endif //CARTOCROW_REGION_MAP_READER_H
