#ifndef CARTOCROW_MOSAIC_CARTOGRAM_OST_H
#define CARTOCROW_MOSAIC_CARTOGRAM_OST_H

#include <vector>
#include <CGAL/Arr_vertex_index_map.h>

#include "../core/core.h"
#include "../core/region_arrangement.h"
#include "graph.h"

namespace cartocrow::mosaic_cartogram {

class RegionArrangementOST;  // forward declaration (so it can be declared as friend below)

namespace detail {

/// A class that computes and represents an orderly spanning tree. On its own, this class is only
/// used for testing.
/// It uses the algorithm described by Schnyder (1990) in sections 4 and 8.
class OrderlySpanningTree {
	friend class cartocrow::mosaic_cartogram::RegionArrangementOST;

  public:
	/// Creates an OST for the given graph, which must be connected and triangular (i.e., maximal
	/// planar). Since the graph data structure does not specify an embedding, the three vertices on
	/// the outer boundary must also be specified.
	OrderlySpanningTree(const Graph &g, int r1, int r2, int r3);

	int getRoot() const {
		return m_root1;
	}

	/// Returns the parent of the given vertex w.r.t. the OST. A vertex is its own parent if and
	/// only if it's the root.
	int getParent(int v) const {
		return m_parent[v];
	}

  private:
	Graph m_graph;
	const int m_root1, m_root2, m_root3;  // .. of the red, blue, and green trees

	/// This array defines the red tree: it specifies for each vertex its parent. Note that we are
	/// not interested in the blue or green tree.
	/// At initialization, it's filled with \c m_root1. In \c contract, we only update the array for
	/// vertices that have a different parent.
	std::vector<int> m_parent;

	void contract(const int n);
};

} // namespace detail

// TODO: why doesn't `operator[]` accept `const Vertex_handle`? then `arr` etc. could be const!
template <typename A> using IndexMap = CGAL::Arr_vertex_index_map<A>;

class RegionArrangementOST {
  public:
	using Vertex_handle = RegionArrangement::Vertex_handle;

	explicit RegionArrangementOST(RegionArrangement &arr)
	    : m_arr(arr), m_idxMap(arr), m_ost(arrangementToOST(arr, m_idxMap)) {}

	/// Returns the vertex in the arrangement that corresponds to the root of the OST.
	Vertex_handle getRoot() const {
		return m_idxMap.vertex(m_ost.m_root1);
	}

	/// Given a vertex in the arrangement, returns its parent w.r.t. the OST. A vertex is its own
	/// parent if and only if it's the root.
	Vertex_handle getParent(const Vertex_handle v) const {
		int i = m_idxMap[v];
		return m_idxMap.vertex(m_ost.m_parent[i]);
	}

  private:
	const RegionArrangement &m_arr;
	const IndexMap<RegionArrangement> m_idxMap;
	const detail::OrderlySpanningTree m_ost;

	static detail::OrderlySpanningTree arrangementToOST(RegionArrangement &arr, const IndexMap<RegionArrangement> &idxMap);
};

} // namespace cartocrow::mosaic_cartogram

#endif //CARTOCROW_MOSAIC_CARTOGRAM_OST_H
