#pragma once

#include "../../core/core.h"
#include "../../core/region_arrangement.h"
#include "vertex_removal.h"

namespace cartocrow::simplification {

struct VWVertex;

struct VWTraits {
	using Map = RegionArrangement<VWVertex>;

	static void vrSetCost(Map::Vertex_handle v, Triangle<Exact> T);
	static Number<Exact> vrGetCost(Map::Vertex_handle v);

	static void vrSetBlockingNumber(Map::Vertex_handle v, int b);
	static int vrGetBlockingNumber(Map::Vertex_handle v);

	static void vrSetHalfedge(Map::Vertex_handle v, Map::Halfedge_handle inc);
	static Map::Halfedge_handle vrGetHalfedge(Map::Vertex_handle v);
};

struct VWVertex {
	int block;
	Number<Exact> cost;
	VWTraits::Map::Halfedge_handle inc;
};

using VWSimplification = VertexRemovalSimplification<VWTraits>;

} // namespace cartocrow::simplification
