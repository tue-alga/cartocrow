#pragma once

#include "../../core/core.h"
#include "../../core/region_arrangement.h"
#include "../historic_arrangement.h"
#include "../oblivious_arrangement.h"
#include "vertex_removal.h"

/// Paper title:
///   Line generalisation by repeated elimination of points
/// Paper authors:
///	  M. Visvalingam
///	  J. D. Whyatt
/// doi:
///   https://doi.org/10.1179/000870493786962263
///
/// Implementation notes:
///    Implemented through \ref VWTraits that realizes the concepts \ref VertexRemovalTraits and \ref EdgeStoredHistory.
///    The cost of an operation is set to the area of the spanned triangle, and the algorithm is made topologically safe.
///    Shorthands for Visvalingam-Whyatt simplification, with and without, history are provided.

namespace cartocrow::simplification {

struct VWVertex;
struct VWEdge;

/// Implements the \ref VertexRemovalTraits concept
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

using VWSimplification = VertexRemovalSimplification<ObliviousArrangement<VWTraits>, VWTraits>;
using VWSimplificationWithHistory =
    VertexRemovalSimplification<HistoricArrangement<VWTraits>, VWTraits>;

struct VWVertex {
	int block;
	Number<Exact> cost;
	VWTraits::Map::Halfedge_handle inc;
};

struct VWEdge {
	EdgeHistory<VWTraits>* hist = nullptr;
};

} // namespace cartocrow::simplification
