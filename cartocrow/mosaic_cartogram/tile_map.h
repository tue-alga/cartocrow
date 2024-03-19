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

	struct CoordinateHash;  // forward declaration

	/// The radius of the inscribed circle of the unit hexagon (i.e., the largest circle that fits).
	/// Also known as the apothem.
	static constexpr Number<Inexact> tileInradius = 0.537284965911770959776669078352965289L;
	/// The radius of the circumcircle of the unit hexagon (i.e., the circle that passes through all
	/// six vertices). Also known as the circumradius, and equal to the side length.
	static constexpr Number<Inexact> tileExradius = 0.620403239401399732627479164655489392L;

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

	template<typename T>
	using CoordinateMap = std::unordered_map<Coordinate, T, CoordinateHash>;

	/// Invariant: all tiles are connected and there are no holes. We say the configuration is \a contiguous.
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
		bool isLand() const {
			return region.has_value();
		}
		bool isSea() const {
			return !region.has_value();
		}
		std::string label() const {
			return region ? region->get().name : "sea";
		}
		int size() const {
			return tiles.size();
		}

		bool containsInteriorly(Coordinate c) const;
		bool isAdjacent(Coordinate c) const;
		bool remainsContiguousWithout(Coordinate c) const;
	};

	struct Transfer {
		Coordinate tile;
		int targetIndex;
		double score;  // lower is better

		bool operator<(const Transfer &t) const {
			return score < t.score;
		}
	};

	std::vector<std::unordered_map<int, GuidingPair>> guidingPairs;

	std::vector<Configuration> configurations;

	/// The mapping from tiles (coordinates) to the configurations they belong to.
	CoordinateMap<int> tiles;

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

	int getNumberOfLandRegions() const { return guidingPairs.size() + 1; }
	int getNumberOfSeaRegions() const { return configurations.size() - getNumberOfLandRegions(); }

	// TODO: `opt_ref` instead of pointer?
	const Configuration* getConfiguration(Coordinate c) const;
	Configuration* getConfiguration(Coordinate c);

	/// Get the guiding shape for a land region.
	Ellipse getGuidingShape(const Configuration &config) const;
	/// Get the pair of guiding shapes for two adjacent land regions. This is not equivalent to
	/// getting two separate guiding shapes! This pair has the correct relative positions (of the
	/// original regions) and is centered on the joint centroid.
	std::pair<Ellipse, Ellipse> getGuidingPair(const Configuration &config1, const Configuration &config2) const;
	/// Get the pair of guiding shapes for any two regions.
	std::pair<std::optional<Ellipse>, std::optional<Ellipse>> getGuidingShapes(const Configuration &config1, const Configuration &config2) const;

	std::vector<Coordinate> computeTransferCandidates(const Configuration &source, const Configuration &target) const;
	std::vector<Transfer> computeAllTransfers(const Configuration &source, const Configuration &target) const;
	std::optional<Transfer> computeBestTransfer(const Configuration &source, const Configuration &target) const;
	std::optional<Transfer> computeBestTransfer() const;

	void perform(const Transfer &transfer);

};

} // namespace cartocrow::mosaic_cartogram

#endif // CARTOCROW_MOSAIC_CARTOGRAM_MAP_H