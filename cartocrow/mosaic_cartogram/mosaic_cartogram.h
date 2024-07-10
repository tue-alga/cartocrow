#ifndef CARTOCROW_MOSAIC_CARTOGRAM_MOSAIC_CARTOGRAM_H
#define CARTOCROW_MOSAIC_CARTOGRAM_MOSAIC_CARTOGRAM_H

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../core/core.h"
#include "../core/ellipse.h"
#include "../core/region_arrangement.h"
#include "../core/region_map.h"
#include "graph.h"
#include "parameters.h"
#include "region.h"
#include "tile_map.h"

namespace cartocrow::mosaic_cartogram {

/// The entrypoint for computing mosaic cartograms.
/// First, we transform the input map into a set of contiguous land regions, each with a desired
/// number of tiles (\ref computeLandRegions).
/// Next, we add sea regions, either manually from the input map, or computed using a straight
/// skeleton (\ref computeArrangement).
/// Then, we compute the dual of the processed regions, where nodes corresponds to regions and edges
/// to adjacencies (\ref computeDual).
/// Finally, we compute a \ref VisibilityDrawing that will serve as an initial tile map
/// (\ref computeTileMap). Computation then carries on in \ref HexagonalMap.
class MosaicCartogram {
	friend class Painting;

  public:
	/// Constructs a mosaic cartogram with the given regions.
	///
	/// This does not compute the mosaic cartogram: use \ref compute() to run
	/// the computation. Modifying the RegionMap passed here after the mosaic
	/// cartogram has been constructed results in undefined behavior.
	MosaicCartogram(
		std::shared_ptr<RegionMap> map,
		std::unordered_map<std::string, Number<Inexact>> &&dataValues,
		std::vector<Point<Exact>> &&salientPoints  // (temp) should be computed automatically from the input map
	) : m_inputMap(map),
	    m_dataValues(std::move(dataValues)),
	    m_salientPoints(std::move(salientPoints)) {}

	void compute();
	Parameters& parameters() { return m_parameters; }

  private:
	Parameters m_parameters;
	const std::shared_ptr<RegionMap> m_inputMap;
	/// A mapping from region names (in the input map) to the corresponding user-supplied values.
	/// After \ref computeLandRegions(), these raw values should not be used anymore.
	const std::unordered_map<std::string, Number<Inexact>> m_dataValues;
	/// The points (vertices of land regions) that control the creation of sea regions.
	std::vector<Point<Exact>> m_salientPoints;

	std::vector<LandRegion> m_landRegions;
	std::vector<SeaRegion> m_seaRegions;  // excluding the three outer regions
	/// Maps region names to the corresponding indices.
	std::unordered_map<std::string, int> m_regionIndices;

	RegionArrangement m_arrangement;
	UndirectedGraph m_dual;
	HexagonalMap m_tileMap;

	int getTileCount(double value) const {
		return std::round(value / m_parameters.unitValue);
	}
	/// Fits ellipse to (the boundary of) the given \c polygon, translates it to the origin, scales
	/// it to size \c tileCount, and normalizes the contours.
	template<typename K>
	EllipseAtOrigin computeGuidingShape(const PolygonWithHoles<K> &polygon, int tileCount = 1) const {
		// internally, we define tiles to have unit area
		return Ellipse::fit(polygon.outer_boundary())
			.translateToOrigin()
			.scaleTo(tileCount)
			.normalizeContours();
	}

	int getNumberOfRegions() const {
		return m_regionIndices.size();  // == m_landRegions.size() + m_seaRegions.size() + 3
	}

	/// Step 0. Check parameters, region names, and region data.
	void validate() const;
	/// Step 1. Transforms \c m_inputMap to \c m_landRegions such that each region is contiguous,
	/// i.e., consists of one polygon (possibly with holes). All properties of each land region are
	/// set, except \c neighbors, which is computed during step 3.
	void computeLandRegions();
	/// Step 2. Construct arrangement from the contiguous regions, and create sea regions such that
	/// the dual is triangular.
	void computeArrangement();
	/// Step 3. Create a vertex for each face in the arrangement and connect two vertices if the
	/// corresponding faces are adjacent.
	void computeDual();
	/// Step 4. Compute an initial tile map using a visibility diagram and continue computation in
	/// \ref HexagonalMap using a flow-based algorithm.
	void computeTileMap();

	/// (temp) Fixes the only internal problem in Europe, i.e., Moldova having degree 2. This is
	/// achieved by absorbing Moldova into Ukraine at the end of step 2. It should be replaced by a
	/// generalized solution.
	void absorbMoldova();

	static int parseIntAtEnd(const std::string &s);
};

} // namespace cartocrow::mosaic_cartogram

#endif //CARTOCROW_MOSAIC_CARTOGRAM_MOSAIC_CARTOGRAM_H
