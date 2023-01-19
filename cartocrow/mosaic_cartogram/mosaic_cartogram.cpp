#include "mosaic_cartogram.h"

namespace cartocrow::mosaic_cartogram {

MosaicCartogram::MosaicCartogram(const std::shared_ptr<RegionMap> map)
    : m_map(map), m_dual(DualArrangement(regionMapToArrangement(*map))) {}

Parameters& MosaicCartogram::parameters() {
	return m_parameters;
}

void MosaicCartogram::compute() {
	// TODO

	RegionArrangement arr = regionMapToArrangement(*m_map);
	DualArrangement dual = DualArrangement(arr);
	std::cout << "Primal #faces:    " << arr.number_of_faces() << '\n';
	std::cout << "Primal #vertices: " << arr.number_of_vertices() << '\n';
	std::cout << "Dual #vertices:   " << dual.number_of_vertices() << std::endl;
	std::cout << "Dual #edges:      " << dual.number_of_edges() << std::endl;
}

} // namespace cartocrow::mosaic_cartogram
