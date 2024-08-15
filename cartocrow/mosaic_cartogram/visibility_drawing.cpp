#include "visibility_drawing.h"

#include <algorithm>

namespace cartocrow::mosaic_cartogram {

VisibilityDrawing::VisibilityDrawing(const UndirectedGraph &g, int vTop, int vBotL, int vBotR, const std::vector<Point<Exact>> &centroids)
    : m_graph(g), ost(g, vTop, vBotL, vBotR, centroids) {
	// initialize
	const int n = g.getNumberOfVertices();
	m_levelVertices.resize(n, -1);
	m_levelEdges.resize(n, std::vector<int>(n, -1));
	m_x0.resize(n, -1);
	m_x1.resize(n);
	m_y0.resize(n);
	m_y1.resize(n);

	// compute
	m_levelVertices[vTop] = 1;
	for (const int v : ost.getVerticesInOrder()) getOrComputeLevel(v);
	dfs(vTop, 0, 0);

	// fill grid
	const int w = m_x1[vTop];
	const int h = m_y1[vBotL];
	grid.resize(w, std::vector<int>(h, -1));
	for (int i = 0; i < n; i++)
		for (int x = m_x0[i]; x < m_x1[i]; x++)
			for (int y = m_y0[i]; y < m_y1[i]; y++)
				grid[x][y] = i;

	// extend rectangles into empty cells
	for (int y = 0; y < h; y++) {
		int v = grid[0][y];
		for (int x = 1; x < w; x++)
			if (grid[x][y] == -1)
				grid[x][y] = v;
			else
				v = grid[x][y];
	}
}

std::optional<int> VisibilityDrawing::getUnrelatedNeighborLeft(const int v) const {
	const auto &neighbors = m_graph.getNeighbors(v);
	const int label = ost.getLabel(v);

	const int n = neighbors.size();
	const int iParent = std::find(neighbors.begin(), neighbors.end(), ost.getParent(v)) - neighbors.begin();

	int i = iParent;
	do {
		i = (i-1 + n) % n;
	} while (i != iParent && ost.getLabel(neighbors[i]) < label);  // i.e., `neighbors[i]` is in U_<(v)

	i = (i+1) % n;
	return i == iParent ? std::nullopt : std::optional(neighbors[i]);
}

std::optional<int> VisibilityDrawing::getUnrelatedNeighborRight(const int v) const {
	const auto &neighbors = m_graph.getNeighbors(v);
	const int label = ost.getLabel(v);

	const int n = neighbors.size();
	const int iParent = std::find(neighbors.begin(), neighbors.end(), ost.getParent(v)) - neighbors.begin();

	int i = iParent;
	do {
		i = (i+1) % n;
	} while (ost.getLabel(neighbors[i]) > label && ost.getParent(neighbors[i]) != v);  // i.e., `neighbors[i]` is in U_>(v)

	i = (i-1 + n) % n;
	return i == iParent ? std::nullopt : std::optional(neighbors[i]);
}

int VisibilityDrawing::getOrComputeLevel(const int v) {
	if (m_levelVertices[v] == -1) {
		const auto l = getUnrelatedNeighborLeft(v);
		const auto r = getUnrelatedNeighborRight(v);
		m_levelVertices[v] = std::max(
			// only the root doesn't have both, but its level was already set in advance
			l ? getOrComputeLevel(*l, v) : -1,
			r ? getOrComputeLevel(v, *r) : -1
		);
	}
	return m_levelVertices[v];
}

int VisibilityDrawing::getOrComputeLevel(const int u, const int v) {  // assert: u.label < v.label
	if (m_levelEdges[u][v] == -1) {
		const auto w = m_graph.getNextNeighbor(v, u).value();  // assert: w has value
		m_levelEdges[u][v] = 1 + std::max(
			w == ost.getParent(u) ? getOrComputeLevel(w) : getOrComputeLevel(u, w),
			w == ost.getParent(v) ? getOrComputeLevel(w) : getOrComputeLevel(w, v)
		);
	}
	return m_levelEdges[u][v];
}

int VisibilityDrawing::dfs(const int curr, int x, int y) {
	m_x0[curr] = x;
	m_y0[curr] = y;
	m_y1[curr] = y = m_levelVertices[curr];

	const auto &children = ost.getChildren(curr);
	if (children.empty()) {
		// `curr` is leaf
		x += 1;
	} else {
		// `curr` is internal node
		for (const int next : children)
			if (m_x0[next] == -1)  // skip if `next` was already visited
				x = dfs(next, x, y);
	}

	m_x1[curr] = x;
	return x;
}

}
