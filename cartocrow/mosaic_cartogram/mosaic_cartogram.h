#ifndef CARTOCROW_MOSAIC_CARTOGRAM_MOSAIC_CARTOGRAM_H
#define CARTOCROW_MOSAIC_CARTOGRAM_MOSAIC_CARTOGRAM_H

#include <cctype>
#include <functional>
#include <numbers>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "../core/core.h"
#include "../core/region_arrangement.h"
#include "../core/region_map.h"
#include "ellipse.h"
#include "graph.h"
#include "parameters.h"
#include "region.h"
#include "tile_map.h"

namespace cartocrow::mosaic_cartogram {

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
	const std::unordered_map<std::string, Number<Inexact>> m_dataValues;
	std::vector<Point<Exact>> m_salientPoints;

	std::vector<LandRegion> m_landRegions;
	std::unordered_map<std::string, int> m_landIndices;
	/// The number of sea regions, excluding the three outer regions.
	int m_seaRegionCount;

	RegionArrangement m_arrangement;
	UndirectedGraph m_dual;
	HexagonalMap m_tileMap;

	int getTileCount(double value) const {
		return std::round(value / m_parameters.unitValue);
	}
	Number<Inexact> getTileArea(int n = 1) const {
		return n * std::numbers::pi * m_parameters.tileRadius * m_parameters.tileRadius;
	}
	template <class K>
	EllipseAtOrigin getGuidingShape(const PolygonWithHoles<K> &polygon, int tileCount) const {
		return Ellipse::fit(polygon.outer_boundary())
			.translateToOrigin()
			.scaleTo(getTileArea(tileCount));
	}

	bool isLandRegion(const int index) const {
		return index < m_landRegions.size();
	}
	int getRegionCount() const {
		return m_landRegions.size() + m_seaRegionCount + 3;
	}
	int getRegionIndex(const std::string &name) const {
		// check if name belongs to land region
		const auto it = m_landIndices.find(name);
		if (it != m_landIndices.end()) return it->second;

		// otherwise, it's a sea region (TODO: ugly)
		int i = 0, e = 1;
		for (auto c = name.rbegin(); std::isdigit(*c); ++c) i += e * (*c - '0'), e *= 10;
		return m_landRegions.size() + (name[1] == 'o' ? m_seaRegionCount : 0) + i;
	}
	std::string getRegionName(const int index) const {
		if (index < 0) throw std::out_of_range("No region exists with index " + std::to_string(index));

		// land
		if (isLandRegion(index)) return m_landRegions[index].name;

		// ordinary sea
		int i = index - m_landRegions.size();
		if (i < m_seaRegionCount) return "_sea" + std::to_string(i);

		// outer sea
		i -= m_seaRegionCount;
		if (i < 3) return "_outer" + std::to_string(i);

		throw std::out_of_range("No region exists with index " + std::to_string(index));
	}

	/// Step 2. Construct arrangement from the contiguous regions, and create sea regions such that
	/// the dual is triangular.
	void computeArrangement();
	/// Step 3. Create a vertex for each face in the arrangement and connect two vertices if the
	/// corresponding faces are adjacent.
	void computeDual();
	/// Step 1. Transforms \c m_inputMap to \c m_landRegions such that each region is contiguous,
	/// i.e., consists of one polygon (possibly with holes).
	void computeLandRegions();
	/// Step 4. Compute an initial tile map using a visibility diagram and TODO.
	void computeTileMap();
	/// Step 0. Check parameters, region names, and region data.
	void validate() const;
};

} // namespace cartocrow::mosaic_cartogram

#endif //CARTOCROW_MOSAIC_CARTOGRAM_MOSAIC_CARTOGRAM_H
