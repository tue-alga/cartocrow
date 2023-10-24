#ifndef CARTOCROW_MOSAIC_CARTOGRAM_MAP_H
#define CARTOCROW_MOSAIC_CARTOGRAM_MAP_H

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "../core/core.h"
#include "../renderer/geometry_renderer.h"
#include "ellipse.h"
#include "guiding_shape.h"
#include "parameters.h"
#include "region.h"
#include "visibility_drawing.h"

namespace cartocrow::mosaic_cartogram {

// TODO: make base class and separate subclass SquareMap

class HexagonalMap {
  public:
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
	  private:
		int m_x, m_y;

	  public:
		int x() const { return m_x; }
		int y() const { return m_y; }
		int z() const { return 0; }

		Coordinate() {}
		Coordinate(int x, int y, int z = 0) : m_x(x-z), m_y(y-z) {}

		bool operator==(const Coordinate &c) const { return m_x == c.m_x && m_y == c.m_y; }
	};

	struct CoordinateHash {
		std::size_t operator()(const Coordinate &c) const noexcept {
			return (17 * 31 + c.x()) * 31 + c.y();
		}
	};

	struct Configuration {
		std::optional<std::reference_wrapper<const LandRegion>> region;
		std::unordered_set<Coordinate, CoordinateHash> tiles;

		int id() const { return region ? region->get().id : -1; }
		bool isSea() const { return !region.has_value(); }
	};

  private:
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

  public:
	HexagonalMap() {}
	HexagonalMap(const VisibilityDrawing &initial, const std::vector<LandRegion> &landRegions,
	             int seaRegionCount, const Parameters &params);

	void paint(renderer::GeometryRenderer &renderer) const;
	void paintTile(renderer::GeometryRenderer &renderer, const Coordinate &c) const;

	std::pair<Ellipse, Ellipse> getGuidingPair(const Configuration &config1, const Configuration &config2) const;

  private:
	Point<Inexact> getCentroid(const Configuration &config) const;
	Point<Inexact> getCentroid(const Coordinate &c) const;
};

} // namespace cartocrow::mosaic_cartogram

#endif // CARTOCROW_MOSAIC_CARTOGRAM_MAP_H
