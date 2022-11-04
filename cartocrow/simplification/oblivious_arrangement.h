#pragma once

#include "../core/core.h"
#include "common_arrangement.h"
#include "common_traits.h"

/// Definitions for an ObliviousArrangement that does not keep track of the performed operations.
namespace cartocrow::simplification {

// implements the ModifiableArrangement concept
template <MapType MT> class ObliviousArrangement {
  public:
	using Map = MT::Map;

	ObliviousArrangement(Map& inmap) : map(inmap){};

	~ObliviousArrangement() {}

	Map::Halfedge_handle mergeWithNext(Map::Halfedge_handle e, Number<Exact> cost) {
		return merge_with_next(map, e);
	}

	Map& getMap() {
		return map;
	}

  private:
	Map& map;
};

} // namespace cartocrow::simplification