#ifndef CARTOCROW_MOSAIC_CARTOGRAM_GRAPH_H
#define CARTOCROW_MOSAIC_CARTOGRAM_GRAPH_H

#include <vector>

namespace cartocrow::mosaic_cartogram {

/// A simple, undirected graph with a fixed number of vertices. Many operations are not available,
/// simply because they are not used by the OST algorithm.
class Graph {
  public:
	/// Creates a graph consisting of \c n vertices and no edges.
	/// Runtime: O(n)
	explicit Graph(int n) : m_adj(n) {}

	/// Runtime: O(1)
	int getNumberOfVertices() const {
		return m_adj.size();
	}

	/// Runtime: O(1)
	const std::vector<int>& getNeighbors(int v) const {
		return m_adj[v];
	}

	/// Runtime: O(deg(u) deg(v))
	int getNumberOfCommonNeighbors(int u, int v) const;

	/// Adds an undirected edge between \c u and \c v, even if it violates the class invariants.
	/// Runtime: O(1)
	void addEdgeUnsafe(int u, int v) {
		m_adj[u].push_back(v);
		m_adj[v].push_back(u);
	}

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
	/// - <tt>!m_adj[u].contains(u)</tt>
	/// - <tt>m_adj[u].contains(v)</tt> iff <tt>m_adj[v].contains(u)</tt>
	/// - <tt>m_adj[u].count(v) <= 1</tt>
	/// Note that the vectors are unordered! (i.e., the class doesn't specify an embedding)
	std::vector<std::vector<int>> m_adj;

	void removeDirectedEdge(int u, int v);
};

} // namespace cartocrow::mosaic_cartogram

#endif // CARTOCROW_MOSAIC_CARTOGRAM_GRAPH_H
