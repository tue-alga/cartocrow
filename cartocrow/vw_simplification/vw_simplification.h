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

#include "../core/core.h"

namespace cartocrow::vw_simplification {

struct VWPoint {

	VWPoint(const Point<Exact>& pt);

	Point<Exact> pt;
	int removedAt;
	Number<Exact> cost;
};

/// A class to perform Visvalingam-Whyatt simplification.
class VWSimplification {

  public:
	/// Constructs a simplification for a sequence of points.
	/// \param pts The sequence of 2D points.
	VWSimplification(std::shared_ptr<std::vector<Point<Exact>>> pts);
	~VWSimplification();

	Number<Exact> constructAtComplexity(const int k);

  private:
	void continueToComplexity(const int k);
	void recomputeCost(const int i);

	std::shared_ptr<std::vector<Point<Exact>>> m_input;
	std::vector<VWPoint*> m_complete;
	std::vector<VWPoint*> m_current;
};

} // namespace cartocrow::vw_simplification

#endif //CARTOCROW_VWSIMPL_VW_H
