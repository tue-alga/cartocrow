#include "orderly_spanning_tree.h"

#include <algorithm>
#include <numeric>

namespace cartocrow::mosaic_cartogram {

OrderlySpanningTree::OrderlySpanningTree(const UndirectedGraph &g, int r1, int r2, int r3, const std::vector<Point<Exact>> &positions)
    : m_graph(g), m_root1(r1), m_root2(r2), m_root3(r3), m_positions(positions), m_gc(g),
      m_parent(g.getNumberOfVertices(), r1), m_tree(g.getNumberOfVertices()),
      m_treePreordering(g.getNumberOfVertices()) {
	// TODO: assert that `g` is not too small, connected, triangular, etc.
	contract(g.getNumberOfVertices());
	buildTree(r1);
}

std::vector<int> OrderlySpanningTree::getVerticesInOrder() const {
	std::vector<int> vs(m_graph.getNumberOfVertices());
	std::iota(vs.begin(), vs.end(), 0);  // fill `vs` with 0, .., n-1
	std::sort(vs.begin(), vs.end(), [&](int u, int v) {
		return m_treePreordering[u] < m_treePreordering[v];
	});
	return vs;
}

void OrderlySpanningTree::contract(const int n) {
	if (n <= 3) return;  // only the roots remain, so there's no contractible edge => done

	// 1. Find (the best) contractible edge (from `m_root1` to `target`)
	//    TODO: is it more efficient to precompute an expansion sequence using a *canonical ordering*? (see Schnyder 1990, section 8)
	int target = -1;
	for (int v : m_gc.getNeighbors(m_root1)) {
		// we consider all non-root neighbors of `m_root1` that share exactly two neighbors with it
		if (v != m_root2 && v != m_root3 && m_gc.getNumberOfCommonNeighbors(m_root1, v) == 2) {
			// among all candidates, choose the topmost ("the contraction heuristic")
			if (target == -1 || m_positions[v].y() > m_positions[target].y()) {
				target = v;
			}
		}
	}

	// 2. Contract the edge by:
	//    • removing all edges incident to `target` (which effectively removes `target` from the graph)
	//    • for each removed edge `(target, v)`, adding a new edge `(m_root1, v)` if it did not exist before
	std::vector<int> newNeighbors;  // of `m_root1`
	for (int v : m_gc.getNeighbors(target))
		if (m_gc.addEdge(m_root1, v))
			newNeighbors.push_back(v);
	m_gc.isolate(target);  // after the loop to prevent concurrent modification

	// 3. Contract the remaining edges
	contract(n - 1);

	// 4. Assign parents to construct the "next" part of the red spanning tree
	//    Note that this occurs post-order, i.e., we process the "expansions"
	for (int v : newNeighbors) m_parent[v] = target;
}

int OrderlySpanningTree::buildTree(const int v, int label) {
	m_treePreordering[v] = label++;

	const auto &neighbors = m_graph.getNeighbors(v);  // not empty
	const int parent = m_parent[v];

	const int n = neighbors.size();
	int iEnd = n - 1;

	if (parent != v) {
		// below, we want to start after the parent
		// such that the child at index 0 will be the first child after the parent
		while (neighbors[iEnd] != parent) iEnd--;
	} else {
		// if `v` is the root, we mustn't skip the first neighbor
		buildTreeRecur(v, neighbors[iEnd], label);
	}

	// recur for each child
	// `neighbors` is (assumed to be) in clockwise order, but we want to label vertices left to right, so we iterate in reverse order
	int i = iEnd;
	while (true) {
		i = (i-1 + n) % n;
		if (i == iEnd) break;
		buildTreeRecur(v, neighbors[i], label);
	}

	return label;
}

void OrderlySpanningTree::buildTreeRecur(const int v, const int neighbor, int &label) {
	// if `neighbor` is child of `v`, then add edge to `m_tree` and recur (to find children of `neighbor`)
	if (v == m_parent[neighbor]) {
		m_tree.addEdgeUnsafe(v, neighbor);
		label = buildTree(neighbor, label);
	}
}

} // namespace cartocrow::mosaic_cartogram
