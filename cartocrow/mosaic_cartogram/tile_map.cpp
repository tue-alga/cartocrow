#include "tile_map.h"

#include <cmath>
#include <stdexcept>
#include <CGAL/Origin.h>

namespace cartocrow::mosaic_cartogram {

std::array<HexagonalMap::Coordinate, 6> HexagonalMap::Coordinate::neighbors() const {
	// we must use double outer braces to indicate that the inner braces initialize a Coordinate
	return {{
		{ m_x + 1, m_y + 0 },
		{ m_x + 1, m_y + 1 },
		{ m_x + 0, m_y + 1 },
		{ m_x - 1, m_y + 0 },
		{ m_x - 1, m_y - 1 },
		{ m_x + 0, m_y - 1 }
	}};
}

std::ostream& operator<<(std::ostream &os, HexagonalMap::Coordinate c) {
	return os << '(' << c.m_x << ", " << c.m_y << ')';
}

bool HexagonalMap::Configuration::isAdjacent(const Coordinate c) const {
	if (contains(c)) return false;
	for (const Coordinate d : c.neighbors())
		if (boundary.contains(d))
			return true;
	return false;
}

bool HexagonalMap::Configuration::remainsContiguousWithout(const Coordinate c) const {
	// TODO
	return true;
}

HexagonalMap::HexagonalMap(const VisibilityDrawing &initial,
                           const std::vector<LandRegion> &landRegions,
                           const int seaRegionCount)
    : guidingPairs(landRegions.size() - 1),
      configurations(landRegions.size() + seaRegionCount),
      configGraph(configurations.size()) {

	// precompute all guiding pairs
	for (const LandRegion &r1 : landRegions)
		for (const LandRegion &r2 : r1.neighbors)
			if (r1.id < r2.id)
				guidingPairs[r1.id].insert({ r2.id, GuidingPair(r1, r2) });

	// associate each configuration with corresponding region
	for (const LandRegion &r : landRegions) configurations[r.id].region = r;

	// initialize tiles of each configuration
	const int w = initial.grid.size()    - 2;  // the left and right columns contain only outer sea regions, which are ignored
	const int h = initial.grid[0].size() - 2;  // the bottom and top rows ...
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			int i = initial.grid[x+1][h-y];  // vertical flip
			if (i < configurations.size())  // don't add tiles for outer sea regions
				configurations[i].tiles.insert({ x+y, y });  // convert grid coordinate to barycentric coordinate
		}
	}

	// initialize tile -> configuration mapping
	for (int i = 0; i < configurations.size(); i++) {
		configurations[i].index = i;
		for (const Coordinate c : configurations[i]) tiles[c] = i;
	}

	// compute boundary and neighbors of each configuration
	for (Configuration &config : configurations) {
		for (const Coordinate c0 : config) {
			for (const Coordinate c1 : c0.neighbors()) {
				const auto it = tiles.find(c1);
				if (it == tiles.end()) {
					// `c0` has an unassigned neighboring tile
					config.boundary.insert(c0);
				} else if (it->second != config.index) {
					// `c0` has a neighboring tile in another config
					config.boundary.insert(c0);
					configGraph.addEdge(config.index, it->second);
				}
			}
		}
	}
}

Point<Inexact> HexagonalMap::getCentroid(const Coordinate c) {
	return {
		tileInradius * (2*c.x() - c.y()),
		tileExradius * 3/2 * c.y()
	};
}

Point<Inexact> HexagonalMap::getCentroid(const Configuration &config) {
	Vector<Inexact> v(0, 0);
	for (const Coordinate c : config) v += getCentroid(c) - CGAL::ORIGIN;
	return CGAL::ORIGIN + v / config.size();
}

const HexagonalMap::Configuration* HexagonalMap::getConfiguration(const Coordinate c) const {
	const auto it = tiles.find(c);
	return it == tiles.end() ? nullptr : &configurations[it->second];
}

HexagonalMap::Configuration* HexagonalMap::getConfiguration(const Coordinate c) {
	// since `this` is non-const, the following casts are safe
	// https://stackoverflow.com/a/47369227
	return const_cast<Configuration*>(std::as_const(*this).getConfiguration(c));
}

std::pair<Ellipse, Ellipse> HexagonalMap::getGuidingPair(const Configuration &c1, const Configuration &c2) const {
	if (c1.isSea() || c2.isSea())
		throw std::invalid_argument("Sea regions do not have a guiding shape");

	int id1 = c1.id();
	int id2 = c2.id();

	if (id1 == id2)
		throw std::invalid_argument("A guiding pair only exists for distinct regions");

	// compute centroid of union of configurations
	const int size1 = c1.size();
	const int size2 = c2.size();
	const auto centroid = (size1 * (getCentroid(c1) - CGAL::ORIGIN)
	                     + size2 * (getCentroid(c2) - CGAL::ORIGIN)) / (size1 + size2);

	// get precomputed guiding pair and translate to `centroid`
	if (id1 > id2) std::swap(id1, id2);
	const auto p = guidingPairs[id1].at(id2).translate(centroid);  // throws exception if regions are not neighbors
	return id1 == c1.id() ? p : std::make_pair(p.second, p.first);
}

/// Checks whether \c config1 is adjacent to \c config2 via tiles other than \c ignore (which is part of \c config1).
bool adjacent(const HexagonalMap::Configuration &config1, const HexagonalMap::Configuration &config2, const HexagonalMap::Coordinate ignore) {
	for (const HexagonalMap::Coordinate c : config1.boundary)
		if (c != ignore && config2.isAdjacent(c))
			return true;
	return false;
}

std::vector<HexagonalMap::Coordinate> HexagonalMap::getTransferCandidates(const Configuration &source, const Configuration &target) const {
	// the candidates must:
	// 1. be part of `source`
	// 2. be adjacent to `target`
	// 3. not remove neighbors of `source`
	// 4. not add neighbors to `target`
	// 5. not break the contiguity of `source`
	std::vector<Coordinate> candidates;

	// ensure (1), (2), and (5)
	for (const Coordinate &c : source.boundary)
		if (target.isAdjacent(c) && source.remainsContiguousWithout(c))
			candidates.push_back(c);

	// ensure (4)
	std::vector<Coordinate> keep;
	for (const Coordinate c0 : candidates) {
		for (const Coordinate c1 : c0.neighbors()) {
			const auto it = tiles.find(c1);
			if (it == tiles.end()) continue;
			const int neighborIndex = it->second;
			if (neighborIndex == target.index) continue;
			if (!configGraph.containsEdge(target.index, neighborIndex)) {
				// if `c0` is transferred to `target`, there would be an illegal adjacency between `target` and `neighbor`
				goto next4;
			}
		}
		keep.push_back(c0);
	  next4:
		;
	}

	// ensure (3)
	candidates.clear();
	for (const Coordinate c : keep) {
		for (const int neighborIndex : configGraph.getNeighbors(source.index))
			if (neighborIndex != target.index && !adjacent(source, configurations[neighborIndex], c))
				goto next3;  // if `c` is removed from `source`, the adjacency between `source` and `neighbor` would break
		candidates.push_back(c);
	  next3:
		;
	}

	return candidates;
}

std::optional<HexagonalMap::Transfer> HexagonalMap::getBestTransfer(const Configuration &source, const Configuration &target) const {
	if (source.isSea() || target.isSea()) {
		// TODO
		throw std::runtime_error("Transfers between non-land configurations are not yet implemented");
	}

	const std::vector<Coordinate> candidates = getTransferCandidates(source, target);
	if (candidates.empty()) return std::nullopt;

	const auto [gs, gt] = getGuidingPair(source, target);
	const double threshold = 2.0 * source.region->get().targetTileCount;

	const Coordinate *bestTile = nullptr;
	double bestScore;

	for (const Coordinate c : candidates) {
		const Point<Inexact> p = getCentroid(c);
		const double score = std::min(gs.evaluate(p), threshold) - gt.evaluate(p);
		if (!bestTile || score > bestScore) bestTile = &c, bestScore = score;
	}

	return Transfer{*bestTile, bestScore};
}

} // namespace cartocrow::mosaic_cartogram
