#pragma once

#include "../core/core.h"
#include "common_arrangement.h"
#include "common_traits.h"

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

	void goToPresent() {
		// skip
	}

  private:
	Map& map;
};

} // namespace cartocrow::simplification