#include "mosaic_cartogram.h"

namespace cartocrow::mosaic_cartogram {

MosaicCartogram::MosaicCartogram(const std::shared_ptr<RegionMap> map) : m_map(map) {}

Parameters& MosaicCartogram::parameters() {
	return m_parameters;
}

void MosaicCartogram::compute() {
	// TODO
}

} // namespace cartocrow::mosaic_cartogram
