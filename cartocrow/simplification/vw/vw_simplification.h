#pragma once

#include "../../core/arrangement_map.h"
#include "../../core/core.h"
#include "../geometry_store.h"
#include "../iterative_simplification.h"
#include "vw_defs.hpp"

namespace cartocrow::simplification {

class VWSimplification {
  private:
	vw::CDT cdt;
	vw::Queue queue;
	GeometryStore& store; 
	int complexity;

  protected:
	virtual Number<Exact> cost(Triangle<Exact> T);
  public:
	void initialize(BoundaryMap& map, GeometryStore& store);
	void run(StopCriterion stop);
};

/// Convenience function to quickly run VW Simplification
void simplifyVW(BoundaryMap& map, GeometryStore& store, StopCriterion stop) {
	VWSimplification vw();
	vw.initialize(map, store);
	vw.run(stop);
}

} // namespace cartocrow::simplification