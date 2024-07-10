#ifndef CARTOCROW_MOSAIC_CARTOGRAM_VISIBILITY_DRAWING_H
#define CARTOCROW_MOSAIC_CARTOGRAM_VISIBILITY_DRAWING_H

#include <optional>
#include <string>

#include "../core/core.h"
#include "graph.h"
#include "orderly_spanning_tree.h"

namespace cartocrow::mosaic_cartogram {

/// Computes a <em>2-visibility drawing</em> using an <em>orderly-spanning tree</em> like the
/// original paper on mosaic cartograms (Cano et al., 2015) describes.
/// Also refer to section 4 (page 935) of Chiang et al., 2005.
class VisibilityDrawing {
  public:
	// adjacencies of graph must be in clockwise order
	VisibilityDrawing(const UndirectedGraph &g, int vTop, int vBotL, int vBotR, const std::vector<Point<Exact>> &centroids);

	const OrderlySpanningTree ost;
	std::vector<std::vector<int>> grid;

  private:
	const UndirectedGraph &m_graph;

	// memoization
	// level is similar (but not identical) to Chiang (2005), page 935
	std::vector<int> m_levelVertices;
	std::vector<std::vector<int>> m_levelEdges;  // only for edges in Δ-T

	std::vector<int> m_x0, m_x1;
	std::vector<int> m_y0, m_y1;

	/// Returns the first vertex in the block \f$U_<(v)\f$, if it's not empty.
	std::optional<int> getUnrelatedNeighborLeft(const int v) const;
	/// Returns the last vertex in the block \f$U_>(v)\f$, if it's not empty.
	std::optional<int> getUnrelatedNeighborRight(const int v) const;

	int getOrComputeLevel(const int v);
	int getOrComputeLevel(const int u, const int v);  // label(u) < label(v)

	int dfs(const int curr, int x, int y);
};

} // namespace cartocrow::mosaic_cartogram

#endif //CARTOCROW_MOSAIC_CARTOGRAM_VISIBILITY_DRAWING_H
