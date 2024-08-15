#ifndef CARTOCROW_MOSAIC_CARTOGRAM_OST_H
#define CARTOCROW_MOSAIC_CARTOGRAM_OST_H

#include <vector>

#include "../core/core.h"
#include "graph.h"

namespace cartocrow::mosaic_cartogram {

/// A class that computes and represents an orderly spanning tree. It uses the algorithm described
/// by Schnyder (1990) in sections 4 and 8.
class OrderlySpanningTree {
  public:
	/// Creates an OST for the given graph, which must be connected and triangular (i.e., maximal
	/// planar). The three vertices on the outer boundary must also be specified, as well as the
	/// geometric position of each vertex (which is only used for the contraction heuristic).
	OrderlySpanningTree(const UndirectedGraph &g, int r1, int r2, int r3, const std::vector<Point<Exact>> &positions);

	int getNumberOfVertices() const { return m_graph.getNumberOfVertices(); }

	int getRoot() const { return m_root1; }

	/// Returns the parent of the given vertex w.r.t. the OST. A vertex is its own parent if and
	/// only if it's the root.
	int getParent(int v) const { return m_parent[v]; }

	/// Returns the children of the given vertex w.r.t. the OST. They have the same order as in the
	/// original graph.
	const std::vector<int>& getChildren(int v) const { return m_tree.getNeighbors(v); }

	/// Returns the index of the given vertex w.r.t. the counterclockwise preordering of the OST.
	int getLabel(int v) const { return m_treePreordering[v]; }

	/// Returns the list of vertices sorted by their label.
	std::vector<int> getVerticesInOrder() const;

  private:
	const UndirectedGraph m_graph;
	const int m_root1, m_root2, m_root3;  // .. of the red, blue, and green trees
	const std::vector<Point<Exact>> &m_positions;

	/// The contracted graph, only used for computation.
	UndirectedGraph m_gc;

	/// This array defines the red tree: it specifies for each vertex its parent. Note that we are
	/// not interested in the blue or green tree.
	/// At initialization, it's filled with \c m_root1. In \c contract, we only update the array for
	/// vertices that have a different parent.
	std::vector<int> m_parent;

	/// This graph is a more convenient specification of the red tree, computed after \c m_parent.
	Graph m_tree;

	/// Counterclockwise preordering of \c m_tree.
	std::vector<int> m_treePreordering;

	void contract(const int n);

	int buildTree(const int v, int label = 0);
	void buildTreeRecur(const int v, const int neighbor, int &label);
};

} // namespace cartocrow::mosaic_cartogram

#endif //CARTOCROW_MOSAIC_CARTOGRAM_OST_H
