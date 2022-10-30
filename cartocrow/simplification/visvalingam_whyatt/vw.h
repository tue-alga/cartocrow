#pragma once

#include "../../core/core.h"
#include "../../core/region_arrangement.h"
#include "vw_generic.h"

namespace cartocrow::simplification {

struct VWVertex;

struct DefaultVW {
	using Map = RegionArrangement<VWVertex>;

	static Number<Exact> computeCost(Map::Vertex_handle v);
};

struct VWVertex {
	int block;
	Number<Exact> cost;
	DefaultVW::Map::Halfedge_handle inc;

	Triangle<Exact> triangle() {
		return Triangle<Exact>(inc->source()->point(), inc->target()->point(),
		                       inc->next()->target()->point());
	}
};

Number<Exact> DefaultVW::computeCost(Map::Vertex_handle v) {
	return v->data().triangle().area();
}

using VWSimplificationArea = VWSimplification<DefaultVW>;

} // namespace cartocrow::simplification
