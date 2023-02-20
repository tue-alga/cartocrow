#ifndef CARTOCROW_MOSAIC_CARTOGRAM_OST_H
#define CARTOCROW_MOSAIC_CARTOGRAM_OST_H

#include <vector>
#include <CGAL/Arr_vertex_index_map.h>

#include "../core/core.h"
#include "../core/region_arrangement.h"

namespace cartocrow::mosaic_cartogram {

// TODO: why doesn't `operator[]` accept const `Vertex_handle`? then `m_arr` etc. could be const!
template <typename A> using IndexMap = CGAL::Arr_vertex_index_map<A>;

namespace detail {

/// A simple, undirected graph. Many operations are not available, simply because they are not used
/// by the OST algorithm.
class Graph {
  public:
	/// Runtime: O(V + E)
	Graph(RegionArrangement &arr, IndexMap<RegionArrangement> &idxMap);

	/// Runtime: O(1)
	const std::vector<int>& getNeighbors(int v) const {
		return m_adj[v];
	}

	/// Runtime: O(deg(u) deg(v))
	std::vector<int> getCommonNeighbors(int u, int v) const;

	/// Adds an undirected edge between \c u and \c v, unless it already exists or it's a loop.
	/// Runtime: O(deg(u))
	/// \return \c true if and only if an edge was added
	bool addEdge(int u, int v);

	/// Removes the edge between \c u and \c v (if it exists, otherwise there's no effect).
	/// Runtime: O(deg(u) + deg(v))
	void removeEdge(int u, int v) {
		removeDirectedEdge(u, v);
		removeDirectedEdge(v, u);
	}

  private:
	/// Invariants for all <tt>u,v</tt>:
	/// - <tt>m_adj[u].contains(v)</tt> iff <tt>m_adj[v].contains(u)</tt>
	/// - <tt>m_adj[u].count(v) <= 1</tt>
	/// Note that the vectors are unordered! (i.e., the class doesn't specify an embedding)
	std::vector<std::vector<int>> m_adj;

	void addEdgeUnsafe(int u, int v) {
		m_adj[u].push_back(v);
		m_adj[v].push_back(u);
	}

	void removeDirectedEdge(int u, int v);
};

} // namespace detail

class OrderlySpanningTree {
  public:
	using Vertex_handle = RegionArrangement::Vertex_handle;

	/// Computes an orderly spanning tree using Schnyder labeling from a triangular graph
	/// represented by an arrangement.
	explicit OrderlySpanningTree(RegionArrangement &arr);

	/// Returns the root of the orderly spanning tree.
	Vertex_handle getRoot() const {
		return m_idxMap.vertex(m_root1);
	}

	/// Given a vertex in the arrangement, returns its parent in the orderly spanning tree. A vertex
	/// is its own parent if and only if it's the root.
	Vertex_handle getParent(const Vertex_handle v) const {
		int i = m_idxMap[v];
		return m_idxMap.vertex(m_parent[i]);
	}

  private:
	RegionArrangement &m_arr;
	IndexMap<RegionArrangement> m_idxMap;

	detail::Graph m_graph;
	int m_root1, m_root2, m_root3;  // .. of the red, blue, and green trees

	/// This array defines the red spanning tree: it specifies for each vertex its parent. Note that
	/// we are not interested in the blue or green tree.
	std::vector<int> m_parent;

	void contract(const int n);
};

} // namespace cartocrow::mosaic_cartogram

#endif //CARTOCROW_MOSAIC_CARTOGRAM_OST_H
