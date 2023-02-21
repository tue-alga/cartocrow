#include "orderly_spanning_tree.h"

namespace cartocrow::mosaic_cartogram {

namespace detail {

int Graph::getNumberOfCommonNeighbors(int u, int v) const {
	// since the vectors are small, we use a simple quadratic-time approach with little overhead
	int c = 0;
	for (int x : m_adj[u]) {
		for (int y : m_adj[v]) {
			if (x == y) {
				c++;
				break;
			}
		}
	}
	return c;
}

bool Graph::addEdge(int u, int v) {
	// return false if edge is loop or if it already exists
	if (u == v) return false;
	for (int x : m_adj[u])
		if (x == v)
			return false;

	addEdgeUnsafe(u, v);
	return true;
}

void Graph::removeDirectedEdge(int u, int v) {
	std::vector<int> &ns = m_adj[u];
	for (auto it = ns.begin(); it != ns.end(); ++it) {
		if (*it == v) {
			// remove element at `it` without shifting
			*it = ns.back();
			ns.pop_back();
			break;
		}
	}
}

OrderlySpanningTree::OrderlySpanningTree(const Graph &g, int r1, int r2, int r3)
    : m_graph(g), m_root1(r1), m_root2(r2), m_root3(r3), m_parent(g.getNumberOfVertices()) {
	// TODO: assert that `g` is connected, triangular, etc.
	contract(g.getNumberOfVertices());
	m_parent[m_root1] = m_parent[m_root2] = m_parent[m_root3] = m_root1;
}

void OrderlySpanningTree::contract(const int n) {
	if (n < 4) return;  // only the roots remain, so there's no contractible edge => done

	// 1. Find contractible edge (from `m_root1` to `target`)
	//    TODO: it's more efficient to precompute an expansion sequence using a *canonical ordering*? (see Schnyder 1990, section 8)
	int target;
	for (int v : m_graph.getNeighbors(m_root1)) {
		if (v != m_root2 && v != m_root3 && m_graph.getNumberOfCommonNeighbors(m_root1, v) == 2) {
			target = v;  // i.e., we take a non-root neighbor of `m_root1` which shares exactly two neighbors with `m_root1`
			break;
		}
	}

	// 2. Contract the edge by:
	//    • removing all edges incident to `target` (which effectively removes `target` from the graph)
	//    • for each removed edge `(target, v), adding a new edge `(m_root1, v)` if it did not exist before
	std::vector<int> newNeighbors;  // of `m_root1`
	for (int v : m_graph.getNeighbors(target)) {
		m_graph.removeEdge(target, v);
		if (m_graph.addEdge(m_root1, v)) newNeighbors.push_back(v);
	}

	// 3. Contract the remaining edges
	contract(n - 1);

	// 4. Assign parents to construct the "next" part of the red spanning tree
	//    Note that this occurs after the recursive call, so in reverse order (i.e., we process the "expansions")
	m_parent[target] = m_root1;
	for (int v : newNeighbors) m_parent[v] = target;
}

} // namespace detail

detail::OrderlySpanningTree RegionArrangementOST::arrangementToOST(RegionArrangement &arr, const IndexMap<RegionArrangement> &idxMap) {
	detail::Graph g(arr.number_of_vertices());

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
