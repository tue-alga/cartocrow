#ifndef CARTOCROW_MOSAIC_CARTOGRAM_MAP_H
#define CARTOCROW_MOSAIC_CARTOGRAM_MAP_H

#include <array>
#include <functional>
#include <optional>
#include <stdint.h>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "../core/core.h"
#include "../core/ellipse.h"
#include "../renderer/geometry_renderer.h"
#include "guiding_shape.h"
#include "parameters.h"
#include "region.h"
#include "visibility_drawing.h"

namespace cartocrow::mosaic_cartogram {

// TODO: make base class and separate subclass SquareMap

class HexagonalMap {
  public:
	struct CoordinateHash;  // forward declaration

	/// To represent a position in the hexagonal tiling, we use barycentric coordinates. These are
	/// of the form (x,y,z). A step towards the right increases x, towards the top-left increases y,
	/// and towards the bottom-left increases z. Compared to ordinary coordinates, the advantage of
	/// barycentric coordinates is that we can easily move into any direction.
	///
	/// If we move to the right, top-left, and then to the bottom-left, we return to the original
	/// position. Hence, (0,0,0) represents the same position as (1,1,1). In general, (x,y,z) and
	/// (x+a, y+a, z+a) represent the same position. Therefore, we can normalize (x,y,z) to
	/// (x-z, y-z, 0). This class only stores normalized coordinates.
	struct Coordinate {
		friend CoordinateHash;

	  private:
		int m_x, m_y;

	  public:
		int x() const { return m_x; }
		int y() const { return m_y; }
		int z() const { return 0; }

		Coordinate() {}
		Coordinate(int x, int y) : m_x(x), m_y(y) {}
		Coordinate(int x, int y, int z) : Coordinate(x-z, y-z) {}

		bool operator==(const Coordinate &c) const { return m_x == c.m_x && m_y == c.m_y; }

		std::array<Coordinate, 6> neighbors() const;

		friend std::ostream& operator<<(std::ostream &os, const Coordinate &c);
	};

	struct CoordinateHash {
		size_t operator()(const Coordinate &c) const noexcept {
			constexpr int width = SIZE_WIDTH >> 1;
			constexpr int offset = 1 << (width >> 1);

			// size_t is likely (at least) 64 bits
			// and abs(x), abs(y) should fit in 16 bits
			// so it's reasonable to assume that x, y can be packed into a single size_t
			// and otherwise the consequences are not disastrous
			size_t x = c.m_x + offset;
			size_t y = c.m_y + offset;
			return x + (y << width);
		}
	};

	struct Configuration {
		std::optional<std::reference_wrapper<const LandRegion>> region;
		std::unordered_set<Coordinate, CoordinateHash> tiles;

		int desire() const { return region ? region->get().targetTileCount - tiles.size() : 0; }
		int id() const { return region.value().get().id; }  // throws exception if `region` has no value
		bool isSea() const { return !region.has_value(); }
	};

	/// A function that returns a color given a tile coordinate. Used as a parameter for painting,
	/// so different "views" can be drawn with the same function.
	using ColorFunction = std::function<Color(const Coordinate&)>;

	static constexpr Color COLOR_BORDER = { 32,  32,  32};
	static constexpr Color COLOR_LAND   = { 34, 139,  34};  // forest green
	static constexpr Color COLOR_SEA    = {175, 238, 238};  // pale turquoise

	/// The radius of the circumcircle (i.e., the circle that passes through all six vertices). Also
	/// known as the circumradius, and equal to the side length.
	Number<Inexact> exradius;
	/// The radius of the inscribed circle (i.e., the largest circle that can be contained in the
	/// hexagon). Also known as the apothem.
	Number<Inexact> inradius;
	/// The area of one hexagon.
	Number<Inexact> tileArea;
	/// The hexagon used for painting. The bottom left point of its bounding box is at the origin.
	Polygon<Inexact> tileShape;

	std::vector<std::unordered_map<int, GuidingPair>> guidingPairs;

	std::unordered_map<Coordinate, int, CoordinateHash> tiles;
	std::vector<Configuration> configurations;

	HexagonalMap() {}
	HexagonalMap(const VisibilityDrawing &initial, const std::vector<LandRegion> &landRegions,
	             int seaRegionCount, const Parameters &params);

	Configuration& getConfiguration(const Coordinate &c) {
		return configurations[tiles.at(c)];
	}
	const Configuration& getConfiguration(const Coordinate &c) const {
		return configurations[tiles.at(c)];
	}

	Point<Inexact> getCentroid(const Coordinate &c) const;
	Point<Inexact> getCentroid(const Configuration &config) const;

	std::pair<Ellipse, Ellipse> getGuidingPair(const Configuration &config1, const Configuration &config2) const;

	Color getColorDefault(const Coordinate &c) const {
		const Configuration &config = getConfiguration(c);
		return config.isSea() ? Color{255, 255, 255} : config.region->get().color;
	}
	Color getColorUniform(const Coordinate &c) const {
		return getConfiguration(c).isSea() ? COLOR_SEA : COLOR_LAND;
	}

	// TODO: move to painting.cpp
	void paintTile(renderer::GeometryRenderer &renderer, const Coordinate &c) const;
	void paint(renderer::GeometryRenderer &renderer, ColorFunction tileColor) const;
	void paint(renderer::GeometryRenderer &renderer) const {
		paint(renderer, std::bind(&HexagonalMap::getColorUniform, this, std::placeholders::_1));
	}
};

} // namespace cartocrow::mosaic_cartogram

#endif // CARTOCROW_MOSAIC_CARTOGRAM_MAP_H
