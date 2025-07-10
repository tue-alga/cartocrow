#pragma once

#include "../core/core.h"
#include <filesystem>

namespace cartocrow {

class Boundary {
  public:
	std::vector<Point<Exact>> points;
	bool closed;
};

class BoundaryMap {
  public:
	std::vector<Boundary> boundaries;		
};
} // namespace cartocrow