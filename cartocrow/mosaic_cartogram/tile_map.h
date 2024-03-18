#ifndef CARTOCROW_MOSAIC_CARTOGRAM_MAP_H
#define CARTOCROW_MOSAIC_CARTOGRAM_MAP_H

#include <array>
#include <functional>
#include <optional>
#include <ostream>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "../core/core.h"
#include "../core/ellipse.h"
#include "graph.h"
#include "guiding_shape.h"
#include "region.h"
#include "visibility_drawing.h"

namespace cartocrow::mosaic_cartogram {

// TODO: make base class and separate subclass SquareMap
// TODO: fix member visibilities and befriend Painting

class HexagonalMap {
  public:

	template<typename T>
	using opt_ref = std::optional<std::reference_wrapper<T>>;

	/// The radius of the inscribed circle of the unit hexagon (i.e., the largest circle that fits).
	/// Also known as the apothem.
	static constexpr Number<Inexact> tileInradius = 0.537284965911770959776669078352965289L;
	/// The radius of the circumcircle of the unit hexagon (i.e., the circle that passes through all
	/// six vertices). Also known as the circumradius, and equal to the side length.
	static constexpr Number<Inexact> tileExradius = 0.620403239401399732627479164655489392L;

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
	///
	/// This class is immutable.
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

		bool operator==(Coordinate c) const {
			return m_x == c.m_x && m_y == c.m_y;
		}

		std::array<Coordinate, 6> neighbors() const;

		friend std::ostream& operator<<(std::ostream &os, Coordinate c);
	};

	struct CoordinateHash {
		size_t operator()(Coordinate c) const noexcept {
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
		using TileSet = std::unordered_set<Coordinate, CoordinateHash>;

		int index;
		opt_ref<const LandRegion> region;
		TileSet tiles, boundary;

		// implement iterators so we can use foreach loops
		TileSet::const_iterator begin() const noexcept { return tiles.begin(); }
		TileSet::const_iterator   end() const noexcept { return tiles.end();   }

		bool contains(Coordinate c) const {
			return tiles.contains(c);
		}
		int desire() const {
			return region ? region->get().targetTileCount - tiles.size() : 0;
		}
		int id() const {
			return region.value().get().id;  // throws exception if `region` has no value
		}
		bool isSea() const {
			return !region.has_value();
		}
		int size() const {
			return tiles.size();
		}

		bool isAdjacent(Coordinate c) const;
		bool remainsContiguousWithout(Coordinate c) const;
	};

	struct Transfer {
		Coordinate tile;
		double score;  // higher is better
	};

	std::vector<std::unordered_map<int, GuidingPair>> guidingPairs;

	std::vector<Configuration> configurations;

	/// The mapping from tiles (coordinates) to the configurations they belong to.
	std::unordered_map<Coordinate, int, CoordinateHash> tiles;

	/// A graph that stores the adjacencies between configurations: there is an edge (u,v) iff the
	/// u-th configuration is adjacent to the v-th configuration. This also stores adjacencies to
	/// and from sea regions, in contrast to \ref LandRegion::neighbors.
	UndirectedGraph configGraph;

	HexagonalMap() {}
	HexagonalMap(const VisibilityDrawing &initial,
	             const std::vector<LandRegion> &landRegions,
	             int seaRegionCount);

	static Point<Inexact> getCentroid(Coordinate c);
	static Point<Inexact> getCentroid(const Configuration &config);

	// TODO: `opt_ref` instead of pointer?
	const Configuration* getConfiguration(Coordinate c) const;
	Configuration* getConfiguration(Coordinate c);

	std::pair<Ellipse, Ellipse> getGuidingPair(const Configuration &config1, const Configuration &config2) const;

	std::vector<Coordinate> getTransferCandidates(const Configuration &source, const Configuration &target) const;
	std::optional<Transfer> getBestTransfer(const Configuration &source, const Configuration &target) const;

	/*
	void transfer(const Coordinate &c, Configuration &target);
	*/
};

} // namespace cartocrow::mosaic_cartogram

#endif // CARTOCROW_MOSAIC_CARTOGRAM_MAP_H
