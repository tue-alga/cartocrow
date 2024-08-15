#ifndef CARTOCROW_MOSAIC_CARTOGRAM_MAP_H
#define CARTOCROW_MOSAIC_CARTOGRAM_MAP_H

#include <array>
#include <cassert>
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
// TODO: if there are not valid transfer paths, find a cycle instead

/// A class that represents a tile map (in particular with hexagonal tiles; square tiles are TODO).
/// The main function \ref run() iteratively improves the map through <em>augmenting paths</em>. The
/// image below is the output when run on <tt>europe-population-mosaic-manual-sea.json</tt>.
///
/// \section Problems
/// The implementation of the flow algorithm is a work-in-progress, and has several issues or
/// missing features. The main cause for issues is that, on the one hand, sea regions should have
/// few constraints to give land regions more flexibility and since they do not represent data, but
/// on the other hand, sea regions needs constraints to ensure the algorithm works properly.
/// - Since sea regions do not have a desired number of tiles, they tend to become "snakes" (e.g.,
///   consider the English Channel below). Then, however, they do not allow transfers anymore, since
///   they would make the region discontiguous. Therefore, sea regions need to stay "fat".
/// - Two adjacent land regions use guiding pairs for their cost calculation, but sea regions simply
///   use guiding shapes centered at the region's centroid. As a result, the placement of islands
///   is not enforced in any way (e.g., consider Sicily below).
/// - If the cost graph (see \ref computeBestTransferPath) is disconnected, it seems the current
///   implementation of Edmonds' algorithm does not produce the transfers you'd expect (e.g.,
///   consider Russia below). The exact reason for this is unknown and needs to be investigated.
///   Note in general that Edmonds' algorithm was not the preferred choice of approach, but a]
///   compromise, since finding minimum-length <em>simple</em> paths is NP-hard.
/// - Currently, if you allow transfer paths where both the source and target are sea regions, the
///   algorithm does not converge.
/// - In the end, when all regions have their desired number of tiles, resolving paths would worsen
///   the solution. However, cost-wise, you may still be able to improve the map. Therefore, you
///   should compute <em>transfer cycles</em> that maintain all tile counts, but improve shape,
///   position, etc. This has not been implemented.
///
/// \image html mosaic-cartogram-europe.svg
class HexagonalMap {
  public:
	struct CoordinateHash;  // forward declaration

	/// Sea regions, like land regions, also have guiding shapes, to prevent snaking. However, the
	/// shape of land regions is much more important, so costs for sea regions are multiplied by
	/// this constant.
	static constexpr double seaCostMultiplier = 5;

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
	using CoordinateSet = std::unordered_set<Coordinate, CoordinateHash>;

	/// Invariant: all tiles are connected and there are no holes. We say the configuration is \a contiguous.
	struct Configuration {
		int index;
		const MosaicRegion *region;
		CoordinateSet tiles, boundary;
		/// Whether this configuration is adjacent to the edge of the map.
		bool atHorizon;

		// implement iterators so we can use foreach loops
		CoordinateSet::const_iterator begin() const noexcept { return tiles.begin(); }
		CoordinateSet::const_iterator   end() const noexcept { return tiles.end();   }

		bool contains(Coordinate c) const {
			return tiles.contains(c);
		}
		int desire() const {
			return isLand() ? landRegion().targetTileCount - size() : 0;
		}
		bool isLand() const {
			return region->type() == RegionType::Land;
		}
		bool isSea() const {
			return region->type() == RegionType::Sea;
		}
		std::string label() const {
			return region->name();
		}
		const LandRegion& landRegion() const {
			assert(isLand());
			return *dynamic_cast<const LandRegion*>(region);
		}
		int size() const {
			return tiles.size();
		}

		bool operator==(const Configuration &config) const { return index == config.index; }

		/// Checks whether \c c and all its neighbors are contained in this configuration. This
		/// function only queries \c tiles, not \c boundary.
		bool containsInInterior(Coordinate c) const;

		bool isAdjacent(Coordinate c) const;

		bool remainsContiguousWithout(Coordinate c) const;
	};

	/// Represents a tile that can be moved from its current configuration to the one indexed by
	/// \c targetIndex at a particular \c cost.
	struct Transfer {
		Coordinate tile;
		int targetIndex;
		double cost;  // lower is better

		bool operator<(const Transfer &t) const {
			return cost < t.cost;
		}
	};

	std::vector<std::unordered_map<int, GuidingPair>> guidingPairs;

	std::vector<Configuration> configurations;

	/// The mapping from tiles (coordinates) to the configurations they belong to.
	CoordinateMap<int> tiles;

	/// A graph that stores the adjacencies between configurations: there is an edge (u,v) iff the
	/// u-th configuration is adjacent to the v-th configuration.
	UndirectedGraph configGraph;

	HexagonalMap() {}
	HexagonalMap(const VisibilityDrawing &initial,
	             const std::vector<LandRegion> &landRegions,
	             const std::vector<SeaRegion> &seaRegions);

	static Point<Inexact> getCentroid(Coordinate c);
	static Point<Inexact> getCentroid(const Configuration &config);

	int getNumberOfLandRegions() const { return guidingPairs.size() + 1; }
	int getNumberOfSeaRegions() const { return configurations.size() - getNumberOfLandRegions(); }

	// TODO: `std::optional<std::reference_wrapper<T>>` instead of pointer?
	const Configuration* getConfiguration(Coordinate c) const;
	Configuration* getConfiguration(Coordinate c);

	/// Get the guiding shape for a land region (scaled to its desired size) or sea region (scaled
	/// to its current size). For the input <tt>europe-population-mosaic.json</tt>, the guiding
	/// shapes of all regions are visualized below.
	/// \image html mosaic-cartogram-guiding-shapes.svg
	Ellipse getGuidingShape(const Configuration &config) const;
	/// Get the pair of guiding shapes for two adjacent land regions. This is not equivalent to
	/// getting two separate guiding shapes! This pair has the correct relative positions (of the
	/// original regions) and is centered on the joint centroid.
	std::pair<Ellipse, Ellipse> getGuidingPair(const Configuration &config1, const Configuration &config2) const;
	/// Get the pair of guiding shapes for any two regions.
	std::pair<Ellipse, Ellipse> getGuidingShapes(const Configuration &config1, const Configuration &config2) const;

	/// Computes the set of tiles that can be transferred from \c source to \c target without
	/// adding/removing adjacencies, creating holes, etc.
	std::vector<Coordinate> computeTransferCandidates(const Configuration &source, const Configuration &target) const;
	/// Computes the set of transfers from \c source to \c target, i.e., the set of tiles that can
	/// be transferred, together with the cost of transferal based on guiding shapes. Lower cost is
	/// better.
	std::vector<Transfer> computeAllTransfers(const Configuration &source, const Configuration &target) const;
	/// Computes the best transfer from \c source to \c target among all valid options. For example,
	/// all candidate transfers from France to Germany are visualized below, with a lighter shade
	/// indicating a lower cost, and the best transfer marked by an X.
	/// \image html mosaic-cartogram-best-transfer.svg
	std::optional<Transfer> computeBestTransfer(const Configuration &source, const Configuration &target) const;
	/// A complex function that aims to compute the best of transfers. First, it computes a cost
	/// graph using \ref computeBestTransfer for each edge in \ref configGraph. Next, for every
	/// source, it computes a minimum branching using Edmonds' algorithm. Then, for each branching,
	/// using a DFS, it finds the shortest path that minimizes the maximum cost among its path.
	std::vector<HexagonalMap::Transfer> computeBestTransferPath() const;

	Configuration& getNearestAdjacent(Coordinate c, const std::unordered_map<int, Point<Inexact>> &centroids);
	/// Clears and recomputes the boundary of \c config.
	void resetBoundary(Configuration &config) const;
	/// Add new tiles to configurations at the horizon until the inner configurations are enclosed
	/// by \c layers of tiles.
	void grow(int layers = 5);

	/// Performs a single transfer between two regions, maintaining their boundary.
	void perform(const Transfer &transfer);
	void perform(const std::vector<Transfer> &path);

	/// Repeatedly runs \ref computeBestTransferPath and \ref perform until there are no valid
	/// transfer paths, or until we reach the iteration limit.
	void run(int iterations);

};

} // namespace cartocrow::mosaic_cartogram

#endif // CARTOCROW_MOSAIC_CARTOGRAM_MAP_H
