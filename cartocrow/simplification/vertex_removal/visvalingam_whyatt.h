#pragma once

#include "../../core/core.h"
#include "../../core/region_arrangement.h"
#include "vertex_removal.h"
#include "../historic_arrangement.h"

namespace cartocrow::simplification {

struct VWVertex;
struct VWEdge;

struct VWTraits {
	using Map = RegionArrangement<VWVertex, VWEdge>;

	static void vrSetCost(Map::Vertex_handle v, Triangle<Exact> T);
	static Number<Exact> vrGetCost(Map::Vertex_handle v);

	static void vrSetBlockingNumber(Map::Vertex_handle v, int b);
	static int vrGetBlockingNumber(Map::Vertex_handle v);

	static void vrSetHalfedge(Map::Vertex_handle v, Map::Halfedge_handle inc);
	static Map::Halfedge_handle vrGetHalfedge(Map::Vertex_handle v);

	static void histSetData(Map::Halfedge_handle e, HistoricArrangement<VWTraits>::EdgeData* data);
	static HistoricArrangement<VWTraits>::EdgeData* histGetData(Map::Halfedge_handle e);
};

struct VWVertex {
	int block;
	Number<Exact> cost;
	VWTraits::Map::Halfedge_handle inc;
};

struct VWEdge {
	HistoricArrangement<VWTraits>::EdgeData* hist = nullptr;
};

using VWSimplification = VertexRemovalSimplification<VWTraits>;

} // namespace cartocrow::simplification
