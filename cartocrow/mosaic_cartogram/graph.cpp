#include "graph.h"

#include <algorithm>

namespace cartocrow::mosaic_cartogram {

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

bool Graph::containsEdge(int u, int v) const {
	auto &ns = m_adj[u];
	return std::find(ns.begin(), ns.end(), v) != ns.end();
}

bool Graph::addEdge(int u, int v) {
	if (u == v || containsEdge(u, v)) return false;
	addEdgeUnsafe(u, v);
	return true;
}

bool Graph::removeEdge(int u, int v) {
	auto &ns = m_adj[u];
	auto it = std::find(ns.begin(), ns.end(), v);  // note that there is at most one occurrence
	if (it == ns.end()) return false;
	ns.erase(it);
	return true;
}

void UndirectedGraph::addEdgeUnsafe(int u, int v) {
	Graph::addEdgeUnsafe(u, v);
	Graph::addEdgeUnsafe(v, u);
}

bool UndirectedGraph::removeEdge(int u, int v) {
	if (Graph::removeEdge(u, v)) {
		// (u,v) existed and we removed it, so now we remove (v,u) as well
		auto &ns = m_adj[v];
		ns.erase(std::find(ns.begin(), ns.end(), u));
		return true;
	} else {
		return false;
	}
}

} // namespace cartocrow::mosaic_cartogram
