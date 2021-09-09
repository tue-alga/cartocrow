/*
The Visvaling-Whyatt package implements the iterative algorithm for simplifying polygonal maps.
Copyright (C) 2021  TU Eindhoven

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

Created by Wouter Meulemans (w.meulemans@tue.nl) on 31-08-2021
*/

#ifndef CARTOCROW_VWSIMPL_VW_H
#define CARTOCROW_VWSIMPL_VW_H

#include <memory>
#include <queue>
#include <vector>

#include "cartocrow/common/core_types.h"

namespace cartocrow {
namespace visvalingam_whyatt {

struct VWPoint {

	VWPoint(const Point& pt);

	Point pt;
	int removedAt;
	Number cost;
}; // struct VWPoint

class VWSimplification {

  public:
	VWSimplification(std::vector<Point>* pts);
	~VWSimplification();

	Number ConstructAtComplexity(const int k);

  private:
	void ContinueToComplexity(const int k);
	void RecomputeCost(const int i);

	std::vector<Point>* input;
	std::vector<VWPoint*> complete;
	std::vector<VWPoint*> current;
}; // class VWSimplification

} // namespace visvalingam_whyatt
} // namespace cartocrow

#endif //CARTOCROW_VWSIMPL_VW_H
