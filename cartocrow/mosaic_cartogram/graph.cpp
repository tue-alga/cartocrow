#include "graph.h"

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

} // namespace cartocrow::mosaic_cartogram
