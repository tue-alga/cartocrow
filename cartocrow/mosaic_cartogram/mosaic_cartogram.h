#ifndef CARTOCROW_MOSAIC_CARTOGRAM_MOSAIC_CARTOGRAM_H
#define CARTOCROW_MOSAIC_CARTOGRAM_MOSAIC_CARTOGRAM_H

#include <string>
#include <unordered_map>
#include <vector>

#include "../core/core.h"
#include "../core/region_arrangement.h"
#include "../core/region_map.h"
#include "graph.h"
#include "parameters.h"

namespace cartocrow::mosaic_cartogram {

class MosaicCartogram {
	friend class Painting;  // TODO: does it matter where it's placed?

  public:
	/// Constructs a mosaic cartogram with the given regions.
	///
	/// This does not compute the mosaic cartogram: use \ref compute() to run
	/// the computation. Modifying the RegionMap passed here after the mosaic
	/// cartogram has been constructed results in undefined behavior.
	MosaicCartogram(const std::shared_ptr<RegionMap> map);

	/// Returns the computation parameters for this mosaic cartogram.
	Parameters& parameters() {
		return m_parameters;
	}

	/// Computes the mosaic cartogram.
	void compute();

  private:
	/// The list of regions that this mosaic cartogram is computed for.
	const std::shared_ptr<RegionMap> m_origMap;

	/// The computation parameters.
	Parameters m_parameters;

	RegionMap m_map;
	RegionArrangement m_arr;
	std::vector<std::string> m_indexToCountry;
	std::unordered_map<std::string, int> m_countryToIndex;
	UndirectedGraph m_dual;

	static std::pair<PolygonWithHoles<Exact>, Number<Exact>> getLargest(const PolygonSet<Exact> &set);
};

} // namespace cartocrow::mosaic_cartogram

#endif //CARTOCROW_MOSAIC_CARTOGRAM_MOSAIC_CARTOGRAM_H
