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

#include "vw_simplification.h"

#include <glog/logging.h>

namespace cartocrow {
namespace visvalingam_whyatt {

/**@class VWSimplification
 * @brief A class to perform Visvalingam-Whyatt simplification.
 */

/**@brief Construct a simplification class for a sequence of points.
 * @param pts the sequence of 2D points.
 */
VWSimplification::VWSimplification(std::vector<Point>* pts) {

	this->input = pts;
	for (Point p : *pts) {
		VWPoint* vp = new VWPoint(p);
		this->complete.push_back(vp);
		this->current.push_back(vp);
	}
	for (int i = 0; i < this->current.size(); i++) {
		RecomputeCost(i);
	}
}

VWSimplification::~VWSimplification() {
	for (VWPoint* vp : this->complete) {
		delete vp;
	}
}

Number VWSimplification::ConstructAtComplexity(const int k) {
	ContinueToComplexity(k);

	this->input->clear();
	Number maxCost = 0;

	for (VWPoint* vp : this->complete) {
		if (vp->removedAt <= k) {
			this->input->push_back(vp->pt);
		} else if (maxCost < vp->cost) {
			maxCost = vp->cost;
		}
	}

	return maxCost;
}

void VWSimplification::ContinueToComplexity(const int k) {
	while (this->current.size() > k) {
		int best = 0;
		for (int i = 1; i < this->current.size() - 1; i++) {
			if (best == 0 || this->current[best]->cost > this->current[i]->cost) {
				best = i;
			}
		}
		this->current[best]->removedAt = this->current.size();
		this->current.erase(this->current.begin() + best);
		RecomputeCost(best - 1);
		RecomputeCost(best);
	}
}
void VWSimplification::RecomputeCost(const int i) {
	if (i == 0 || i == this->current.size() - 1) {
		this->current[i]->cost = -1;
	} else {
		this->current[i]->cost = std::abs(
		    CGAL::area(this->current[i - 1]->pt, this->current[i]->pt, this->current[i + 1]->pt));
	}
}

VWPoint::VWPoint(const Point& pt) : pt(pt), removedAt(-1), cost(0) {}

} // namespace visvalingam_whyatt
} // namespace cartocrow
