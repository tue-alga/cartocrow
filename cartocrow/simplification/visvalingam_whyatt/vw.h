#pragma once

#include "../../core/core.h"
#include "../../core/region_arrangement.h"

namespace cartocrow::simplification {

class VWVertex;

using VWMap = RegionArrangement<VWVertex>;

class VWVertex {	
	int block;
	Number<Exact> cost;
	VWMap::Halfedge_handle inc;

	Triangle<Exact> triangle() {
		return Triangle<Exact>(inc->source()->point(), inc->target()->point(),
		                inc->next()->target()->point());
	}

	friend class VWSimplification;
};

class VWSimplification {

  public:
	VWSimplification(VWMap& inmap);
	~VWSimplification();

	void simplify(const int c, const Number<Exact> t);

  private:
	void initVertex(VWMap::Vertex_handle v);

	Number<Exact> max_cost;
	VWMap& map;
};

} // namespace cartocrow::simplification
