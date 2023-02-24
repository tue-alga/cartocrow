#pragma once

#include "../../core/core.h"
#include "../../core/region_arrangement.h"
#include "../oblivious_arrangement.h"
#include "../historic_arrangement.h"
#include "vertex_removal.h"

namespace cartocrow::simplification {

struct VWVertex;
struct VWEdge;

/// These traits implements the \ref VertexRemovalTraits to create a topologically-safe variant of the Visvalingam-Whyatt algorithm.
/// That is, the cost of removing a vertex is equal to the area spanned by its triangle.
///
/// Title: Line generalisation by repeated elimination of points
/// Authors:
///	- M. Visvalingam
///	- J. D. Whyatt
/// Doi: https://doi.org/10.1179/000870493786962263
struct VWTraits {
	using Map = RegionArrangement<VWVertex, VWEdge>;

	static void vrSetCost(Map::Vertex_handle v, Triangle<Exact> T);
	static Number<Exact> vrGetCost(Map::Vertex_handle v);

	static void vrSetBlockingNumber(Map::Vertex_handle v, int b);
	static int vrGetBlockingNumber(Map::Vertex_handle v);

	static void vrSetHalfedge(Map::Vertex_handle v, Map::Halfedge_handle inc);
	static Map::Halfedge_handle vrGetHalfedge(Map::Vertex_handle v);

	static void histSetData(Map::Halfedge_handle e, EdgeHistory<VWTraits>* data);
	static EdgeHistory<VWTraits>* histGetData(Map::Halfedge_handle e);
};

using VWSimplification = VertexRemovalSimplification<ObliviousArrangement<VWTraits>,VWTraits>;
using VWSimplificationWithHistory = VertexRemovalSimplification<HistoricArrangement<VWTraits>,VWTraits>;

/// The data associated with a vertex in the arrangement used in \ref VWTraits.
struct VWVertex {
	int block;
	Number<Exact> cost;
	VWTraits::Map::Halfedge_handle inc;
};

/// The data associated with a halfedge in the arrangement used in \ref VWTraits.
struct VWEdge {
	EdgeHistory<VWTraits>* hist = nullptr;
};

} // namespace cartocrow::simplification
