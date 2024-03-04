#ifndef CARTOCROW_MOSAIC_CARTOGRAM_GRAPH_H
#define CARTOCROW_MOSAIC_CARTOGRAM_GRAPH_H

#include <optional>
#include <utility>
#include <vector>

namespace cartocrow::mosaic_cartogram {

/// A simple, directed graph with a fixed number of vertices.
class Graph {
	/// Invariants for all <tt>u,v</tt>:
	/// - <tt>!m_adj[u].contains(u)</tt>
	/// - <tt>m_adj[u].count(v) <= 1</tt>

  protected:
	std::vector<std::vector<int>> m_adj;

  public:
	/// Creates an empty graph.
	/// Runtime: O(1)
	Graph() = default;

	/// Creates a graph consisting of \c n vertices and no edges.
	/// Runtime: O(n)
	explicit Graph(int n) : m_adj(n) {}

	/// Runtime: O(1)
	int getNumberOfVertices() const { return m_adj.size(); }

	/// Runtime: O(1)
	int getDegree(int v) const { return m_adj[v].size(); }

	/// Runtime: O(1)
	const std::vector<int>& getNeighbors(int v) const { return m_adj[v]; }

	/// Usually but not necessarily, "next" refers to the clockwise direction.
	/// Runtime: O(deg(v))
	std::optional<int> getNextNeighbor(int v, int neighbor) const;

	/// Runtime: O(deg(u) deg(v))
	int getNumberOfCommonNeighbors(int u, int v) const;

	/// Runtime: O(deg(u))
	bool containsEdge(int u, int v) const;

	/// Runtime: O(|E|)
	std::vector<std::pair<int, int>> getEdges() const;

	/// Adds an edge from \c u to \c v, even if it violates the class invariants.
	/// Runtime: O(1)
	virtual void addEdgeUnsafe(int u, int v) { m_adj[u].push_back(v); }

	/// Adds an edge from \c u to \c v, unless it already exists or it's a self-loop.
	/// Runtime: O(deg(u))
	/// \return whether an edge was added
	bool addEdge(int u, int v);

	/// Removes the edge from \c u to \c v (if it exists, otherwise there's no effect).
	/// Runtime: O(deg(u))
	/// \return whether an edge was removed
	virtual bool removeEdge(int u, int v);

	/// Runtime: O(deg(v) + len(a))
	void setAdjacenciesUnsafe(int v, std::vector<int> a) { m_adj[v] = a; }
};

/// A simple, undirected graph with a fixed number of vertices.
class UndirectedGraph : public Graph {
	/// Additional invariants for all <tt>u,v</tt>:
	/// - <tt>m_adj[u].contains(v)</tt> iff <tt>m_adj[v].contains(u)</tt>

  public:
	/// Creates an empty graph.
	/// Runtime: O(1)
	UndirectedGraph() = default;

	/// Creates a graph consisting of \c n vertices and no edges.
	/// Runtime: O(n)
	explicit UndirectedGraph(int n) : Graph(n) {}

	/// Adds an edge between \c u and \c v, even if it violates the class invariants.
	/// Runtime: O(1)
	void addEdgeUnsafe(int u, int v) override;

	/// Removes the edge between \c u and \c v (if it exists, otherwise there's no effect).
	/// Runtime: O(deg(u) + deg(v))
	/// \return whether an edge was removed
	bool removeEdge(int u, int v) override;

	/// Removes all edges incident to \c v.
	/// Runtime: O(\sum_u deg(u)) where u is a neighbor of v
	void isolate(int v);
};

} // namespace cartocrow::mosaic_cartogram

#endif // CARTOCROW_MOSAIC_CARTOGRAM_GRAPH_H
