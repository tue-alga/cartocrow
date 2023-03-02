#include "orderly_spanning_tree.h"

namespace cartocrow::mosaic_cartogram {

namespace detail {

OrderlySpanningTree::OrderlySpanningTree(const UndirectedGraph &g, int r1, int r2, int r3)
    : m_graph(g), m_root1(r1), m_root2(r2), m_root3(r3), m_gc(g),
      m_parent(g.getNumberOfVertices(), r1), m_tree(g.getNumberOfVertices()) {
	// TODO: assert that `g` is not too small, connected, triangular, etc.
	contract(g.getNumberOfVertices());
	buildTree(r1);
}

void OrderlySpanningTree::contract(const int n) {
	if (n < 4) return;  // only the roots remain, so there's no contractible edge => done

	// 1. Find contractible edge (from `m_root1` to `target`)
	//    TODO: it's more efficient to precompute an expansion sequence using a *canonical ordering*? (see Schnyder 1990, section 8)
	int target;
	for (int v : m_gc.getNeighbors(m_root1)) {
		if (v != m_root2 && v != m_root3 && m_gc.getNumberOfCommonNeighbors(m_root1, v) == 2) {
			target = v;  // i.e., we take a non-root neighbor of `m_root1` which shares exactly two neighbors with `m_root1`
			break;
		}
	}

	// 2. Contract the edge by:
	//    • removing all edges incident to `target` (which effectively removes `target` from the graph)
	//    • for each removed edge `(target, v), adding a new edge `(m_root1, v)` if it did not exist before
	std::vector<int> newNeighbors;  // of `m_root1`
	for (int v : m_gc.getNeighbors(target)) {
		m_gc.removeEdge(target, v);
		if (m_gc.addEdge(m_root1, v)) newNeighbors.push_back(v);
	}

	// 3. Contract the remaining edges
	contract(n - 1);

	// 4. Assign parents to construct the "next" part of the red spanning tree
	//    Note that this occurs post-order, i.e., we process the "expansions"
	for (int v : newNeighbors) m_parent[v] = target;
}

void OrderlySpanningTree::buildTree(const int v) {
	auto &ns = m_graph.getNeighbors(v);  // cannot be empty
	const int p = m_parent[v];

	auto itEnd = ns.begin();
	if (p != v) {
		// below, we want to start after the parent
		// such that the child at index 0 will be the first child after the parent
		while (*itEnd != p) ++itEnd;
	} else {
		// if `v` is the root, we mustn't skip the first neighbor
		buildTreeRecur(v, *itEnd);
	}

	auto it = itEnd;
	while (true) {
		++it;
		if (it == ns.end()) it = ns.begin();  // loop around
		if (it == itEnd) break;               // we reached the end
		buildTreeRecur(v, *it);
	}
}

void OrderlySpanningTree::buildTreeRecur(const int v, const int neighbor) {
	if (v == m_parent[neighbor]) {
		m_tree.addEdgeUnsafe(v, neighbor);
		buildTree(neighbor);  // recur: find children of `neighbor`
	}
}

} // namespace detail

detail::OrderlySpanningTree RegionArrangementOST::arrangementToOST(RegionArrangement &arr, const IndexMap<RegionArrangement> &idxMap) {
	UndirectedGraph g(arr.number_of_vertices());

	// add all edges in the arrangement to the graph
	// each edge is represented by one of its half-edges
	for (auto eit = arr.edges_begin(); eit != arr.edges_end(); ++eit) {
		g.addEdgeUnsafe(idxMap[eit->source()], idxMap[eit->target()]);
	}

	// take the three vertices on the outer boundary as the roots of the spanning trees
	RegionArrangement::Ccb_halfedge_circulator ccb = *arr.unbounded_face()->holes_begin();
	int r3 = idxMap[ccb->target()]; ccb++;
	int r2 = idxMap[ccb->target()]; ccb++;
	int r1 = idxMap[ccb->target()];

	return detail::OrderlySpanningTree(g, r1, r2, r3);
}

} // namespace cartocrow::mosaic_cartogram
