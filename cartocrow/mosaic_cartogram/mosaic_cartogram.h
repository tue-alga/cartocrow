#ifndef CARTOCROW_MOSAIC_CARTOGRAM_MOSAIC_CARTOGRAM_H
#define CARTOCROW_MOSAIC_CARTOGRAM_MOSAIC_CARTOGRAM_H

#include <CGAL/graph_traits_dual_arrangement_2.h>

#include "../core/core.h"
#include "../core/region_arrangement.h"
#include "../core/region_map.h"
#include "parameters.h"

typedef CGAL::Dual<cartocrow::RegionArrangement> DualArrangement;

namespace cartocrow::mosaic_cartogram {

class MosaicCartogram {

  public:
	/// Constructs a mosaic cartogram with the given regions.
	///
	/// This does not compute the mosaic cartogram: use \ref compute() to run
	/// the computation. Modifying the RegionMap passed here after the mosaic
	/// cartogram has been constructed results in undefined behavior.
	MosaicCartogram(const std::shared_ptr<RegionMap> map);

	/// Returns the computation parameters for this mosaic cartogram.
	Parameters& parameters();

	/// Computes the mosaic cartogram.
	void compute();

  private:
	/// The list of regions that this mosaic cartogram is computed for.
	const std::shared_ptr<RegionMap> m_map;
	DualArrangement m_dual;
	/// The computation parameters.
	Parameters m_parameters;

	friend class Painting;
};

} // namespace cartocrow::mosaic_cartogram

#endif //CARTOCROW_MOSAIC_CARTOGRAM_MOSAIC_CARTOGRAM_H
