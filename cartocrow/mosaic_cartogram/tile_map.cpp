#include "tile_map.h"

#include <iomanip>
#include <iterator>
#include <optional>
#include <queue>
#include <stdexcept>
#include <boost/graph/adjacency_list.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <CGAL/Origin.h>

#include "edmonds_optimum_branching.hpp"

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

bool HexagonalMap::Configuration::containsInInterior(const Coordinate c) const {
	if (!contains(c)) return false;
	for (const Coordinate d : c.neighbors())
		if (!contains(d))
			return false;
	return true;
}

bool HexagonalMap::Configuration::isAdjacent(const Coordinate c) const {
	if (contains(c)) return false;
	for (const Coordinate d : c.neighbors())
		if (boundary.contains(d))
			return true;
	return false;
}

bool HexagonalMap::Configuration::remainsContiguousWithout(const Coordinate c) const {
	if (!contains(c)) return true;  // no effect
	if (!boundary.contains(c)) return false;  // this would create a hole

	const auto neighbors = c.neighbors();

	bool open = !contains(neighbors.back());  // check if already open (we count this opening later)
	int openings = 0;
	for (const Coordinate d : neighbors) {
		if (contains(d)) {
			open = false;
		} else if (!open) {
			open = true;
			openings++;
		}
	}

	return openings <= 1;  // otherwise the config would be split in twain (at least)
	                       // if all neighbors are outside, we incorrectly find `openings == 0`, but this still yields the correct conclusion
}

HexagonalMap::HexagonalMap(const VisibilityDrawing &initial,
                           const std::vector<LandRegion> &landRegions,
                           const std::vector<SeaRegion> &seaRegions)
    : guidingPairs(landRegions.size() - 1),
      configurations(landRegions.size() + seaRegions.size()),
      configGraph(configurations.size()) {

	// associate each configuration with corresponding region
	for (const LandRegion &r : landRegions) configurations[r.id].region = &r;
	for (const SeaRegion &r : seaRegions) configurations[r.id + landRegions.size()].region = &r;

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

	// determine which configurations are at the horizon
	for (Configuration &config : configurations) {
		config.atHorizon = false;
		for (const Coordinate c0 : config.boundary)
			for (const Coordinate c1 : c0.neighbors())
				if (!tiles.contains(c1)) {
					config.atHorizon = true;
					assert(config.isSea());
					goto next;
				}
	  next:
		;
	}

	// precompute all guiding pairs
	for (const LandRegion &r1 : landRegions) {
		for (const int i2 : configGraph.getNeighbors(r1.id)) {
			if (r1.id > i2) continue;
			const auto &c2 = configurations[i2];
			if (c2.isSea()) continue;
			guidingPairs[r1.id].insert({ i2, GuidingPair(r1, c2.landRegion()) });
		}
	}

	grow(10);
	run(1000);
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

Ellipse HexagonalMap::getGuidingShape(const Configuration &config) const {
	auto gs = config.region->guidingShape;
	if (config.isSea()) gs = gs.scaleTo(config.size());
	return gs.translate(getCentroid(config) - CGAL::ORIGIN).normalizeSign();
}

std::pair<Ellipse, Ellipse> HexagonalMap::getGuidingPair(const Configuration &c1, const Configuration &c2) const {
	// compute centroid of union of configurations
	const int size1 = c1.size();
	const int size2 = c2.size();
	const auto centroid = (size1 * (getCentroid(c1) - CGAL::ORIGIN)
	                     + size2 * (getCentroid(c2) - CGAL::ORIGIN)) / (size1 + size2);

	int id1 = c1.index;
	int id2 = c2.index;
	if (id1 > id2) std::swap(id1, id2);

	// get precomputed guiding pair and translate to `centroid`
	const auto p = guidingPairs[id1].at(id2).translate(centroid);
	return id1 == c1.index ? p : std::make_pair(p.second, p.first);
}

std::pair<Ellipse, Ellipse> HexagonalMap::getGuidingShapes(const Configuration &c1, const Configuration &c2) const {
	if (!configGraph.containsEdge(c1.index, c2.index))
		throw std::invalid_argument("A guiding pair only exists for two distinct, adjacent regions");

	if (c1.isLand() && c2.isLand())
		return getGuidingPair(c1, c2);
	else
		return { getGuidingShape(c1), getGuidingShape(c2) };
}

/// Checks whether \c config1 is adjacent to \c config2 via tiles other than \c ignore (which is part of \c config1).
bool adjacent(const HexagonalMap::Configuration &config1, const HexagonalMap::Configuration &config2, const HexagonalMap::Coordinate ignore) {
	for (const HexagonalMap::Coordinate c : config1.boundary)
		if (c != ignore && config2.isAdjacent(c))
			return true;
	return false;
}

std::vector<HexagonalMap::Coordinate> HexagonalMap::computeTransferCandidates(const Configuration &source, const Configuration &target) const {
	if (source.index == target.index)
		throw std::invalid_argument("Tiles cannot be transferred from a configuration to itself");

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

std::vector<HexagonalMap::Transfer> HexagonalMap::computeAllTransfers(const Configuration &source, const Configuration &target) const {
	const auto candidates = computeTransferCandidates(source, target);
	if (candidates.empty()) return {};

	std::vector<Transfer> transfers;
	transfers.reserve(candidates.size());

	// compute cost for each candidate
	const auto [guideSource, guideTarget] = getGuidingShapes(source, target);
	for (const Coordinate c : candidates) {
		const Point<Inexact> p = getCentroid(c);
		const double cost = guideTarget.evaluate(p) * (target.isSea() ? seaCostMultiplier : 1)
		                  - guideSource.evaluate(p) / (source.isSea() ? seaCostMultiplier : 1);
		transfers.emplace_back(c, target.index, cost);
	}

	return transfers;
}

std::optional<HexagonalMap::Transfer> HexagonalMap::computeBestTransfer(const Configuration &source, const Configuration &target) const {
	const auto transfers = computeAllTransfers(source, target);
	if (transfers.empty()) return std::nullopt;

	const Transfer *best = nullptr;
	for (const Transfer &t : transfers)
		if (!best || t.cost < best->cost)  // minimize
			best = &t;

	return *best;
}

// https://github.com/atofigh/edmonds-alg/blob/master/doc/examples/sparse-example.cpp
std::vector<HexagonalMap::Transfer> HexagonalMap::computeBestTransferPath() const {
	using BoostGraph = boost::adjacency_list<boost::vecS,
	                                         boost::vecS,
	                                         boost::directedS,
	                                         boost::no_property,
	                                         boost::property<boost::edge_weight_t, double>>;
	using Vertex = boost::graph_traits<BoostGraph>::vertex_descriptor;
	using Edge = boost::graph_traits<BoostGraph>::edge_descriptor;

	const int n = configGraph.getNumberOfVertices();

	// create (cost graph with) vertices
	BoostGraph graph(n);
	boost::property_map<BoostGraph, boost::vertex_index_t>::type vertexIndices = get(boost::vertex_index, graph);
	boost::property_map<BoostGraph, boost::edge_weight_t>::type weights = get(boost::edge_weight, graph);

	// copy vertices to vector
	std::vector<Vertex> vs;
	copy(boost::make_iterator_range(vertices(graph)), std::back_inserter(vs));

	// create edges (which are a subset of the configuration graph, and weighted by cost)
	std::vector transfers(n, std::vector<std::optional<Transfer>>(n));
	for (const auto [i, j] : configGraph.getEdges()) {
		const auto t = computeBestTransfer(configurations[i], configurations[j]);
		if (t) {
			transfers[i][j] = t;
			add_edge(vs[i], vs[j], t->cost, graph);
		}
	}

	// internal structure for paths of transfers
	struct Path {
		std::vector<int> configs;
		double cost;

		Path(const int source) : configs({ source }), cost(-1e10) {}  // creates singleton

		int length() const {
			return configs.size();
		}
		bool operator<(const Path &p) const {
			return length() <= p.length() && cost < p.cost;  // whether `this` is "strictly better" than `p`
		}
	};

	std::optional<Path> best;
	for (int source = 0; source < n; source++) {
		const auto &sourceConfig = configurations[source];

		// if the configuration wants to grow, it cannot be a source
		if (sourceConfig.isLand() && sourceConfig.desire() >= 0) continue;

		// using Edmonds' algorithm, compute minimum branching (a.k.a. spanning arborescence) with `source` as root
		std::vector<Edge> branching;
		edmonds_optimum_branching<false, true, 0>(
			graph, vertexIndices, weights,
			vs.begin() + source, vs.begin() + source + 1,
			std::back_inserter(branching)
		);

		// convert edge list to adjacency list
		std::vector<std::vector<std::pair<int, double>>> adj(n);
		for (const auto &e : branching) {
			adj[boost::source(e, graph)].push_back({ boost::target(e, graph), get(weights, e) });
		}

		// DFS to find the "minimum" path
		std::vector<Path> queue { Path(source) };
		while (!queue.empty()) {
			const Path p = queue.back();
			queue.pop_back();

			// prune
			if (best && p.length() >= best->length()) continue;

			for (const auto [target, cost] : adj[p.configs.back()]) {
				Path q = p;
				q.configs.push_back(target);
				q.cost = std::max(q.cost, cost);

				const auto &targetConfig = configurations[target];
				if (targetConfig.isSea() || targetConfig.desire() > 0)
					if (sourceConfig.isLand() || targetConfig.isLand())  // TODO: bad band-aid fix
						if (!best || q < *best)
							best = q;

				queue.push_back(q);
			}
		}
	}

	// convert path into list of transfers, and return it
	std::vector<Transfer> result;
	if (best) {
		for (int i = 1; i < best->configs.size(); i++) {
			const int a = best->configs[i-1];
			const int b = best->configs[i];
			result.push_back(*transfers[a][b]);
		}
	}
	return result;
}

HexagonalMap::Configuration& HexagonalMap::getNearestAdjacent(
	const Coordinate c0,
	const std::unordered_map<int, Point<Inexact>> &centroids
) {
	int nearest = -1;
	double nearestDistance = 1e10;

	const auto p = getCentroid(c0);
	for (const Coordinate c1 : c0.neighbors()) {
		const auto it = tiles.find(c1);
		if (it == tiles.end() || it->second == nearest) continue;
		const auto d = (p - centroids.at(it->second)).squared_length();
		if (d < nearestDistance) nearest = it->second, nearestDistance = d;
	}

	return configurations.at(nearest);
}

void HexagonalMap::resetBoundary(Configuration &config) const {
	config.boundary.clear();
	for (const Coordinate c0 : config.tiles) {
		for (const Coordinate c1 : c0.neighbors()) {
			const auto it = tiles.find(c1);
			if (it == tiles.end() || it->second != config.index) {
				config.boundary.insert(c0);
				break;
			}
		}
	}
}

void HexagonalMap::grow(const int layers) {
	std::unordered_map<int, Point<Inexact>> centroids;
	CoordinateMap<int> distance;
	std::queue<Coordinate> queue;

	for (const Configuration &config : configurations) {
		if (!config.atHorizon) continue;
		centroids.insert({ config.index, getCentroid(config) });
		for (const Coordinate c0 : config.boundary) {
			for (const Coordinate c1 : c0.neighbors()) {
				if (distance.contains(c1)) continue;
				const auto p = tiles.find(c1);
				if (p == tiles.end() || p->second == config.index) continue;
				distance[c1] = 0;
				queue.push(c1);
			}
		}
	}

	int added = 0;
	while (!queue.empty()) {
		const Coordinate c0 = queue.front(); queue.pop();
		const int d = distance[c0];
		for (const Coordinate c1 : c0.neighbors()) {
			if (distance.contains(c1)) continue;  // already visited
			const auto p = tiles.find(c1);
			if (p == tiles.end()) {
				Configuration &config = getNearestAdjacent(c1, centroids);
				config.tiles.insert(c1);
				tiles[c1] = config.index;
				added++;
			}
			if (p == tiles.end() || configurations[p->second].atHorizon) {
				distance[c1] = d+1;
				if (d+1 < layers) queue.push(c1);
			}
		}
	}

	for (Configuration &config : configurations)
		if (config.atHorizon)
			resetBoundary(config);

	std::cerr << "[info] grew by " << added << " tiles" << std::endl;
}

void HexagonalMap::perform(const Transfer &transfer) {
	Configuration &source = configurations[tiles[transfer.tile]];
	Configuration &target = configurations[transfer.targetIndex];

	tiles[transfer.tile] = target.index;

	source.tiles.erase(transfer.tile);
	source.boundary.erase(transfer.tile);

	target.tiles.insert(transfer.tile);
	target.boundary.insert(transfer.tile);

	// maintain boundary of source and target
	for (const Coordinate c : transfer.tile.neighbors()) {
		const auto it = tiles.find(c);
		if (it == tiles.end()) continue;
		if (it->second == source.index) {
			source.boundary.insert(c);
		} else if (it->second == target.index && target.containsInInterior(c)) {
			target.boundary.erase(c);
		}
	}
}

void HexagonalMap::perform(const std::vector<Transfer> &path) {
	std::cout << "path of length " << path.size() + 1 << " : "
	          << configurations[tiles[path[0].tile]].label();
	for (const Transfer &transfer : path) {
		perform(transfer);
		std::cout << " -> " << configurations[transfer.targetIndex].label();
	}
	std::cout << '\n';
}

void HexagonalMap::run(int iterations) {
	// find and perform transfers
	for (int i = 0; i < iterations; i++) {
		const auto path = computeBestTransferPath();
		if (path.empty()) {
			std::cout << "no more valid transfers!\n";
			break;
		}
		perform(path);
	}

	// print list of regions with wrong size
	std::vector<std::pair<int, std::string>> desires;
	for (const Configuration &config : configurations)
		if (config.isLand() && config.desire())
			desires.push_back({ config.desire(), config.label() });
	std::sort(desires.begin(), desires.end());
	std::cout << '\n';
	for (const auto &[desire, label] : desires) {
		std::cout << std::setfill(' ') << std::setw(5) << std::left << label << " : "
		          << std::setw(4) << std::right << desire << '\n';
	}
	std::cout << std::flush;
}

} // namespace cartocrow::mosaic_cartogram
