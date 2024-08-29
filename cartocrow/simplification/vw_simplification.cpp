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

namespace cartocrow::vw_simplification {

VWSimplification::VWSimplification(std::shared_ptr<std::vector<Point<Exact>>> pts) {
	this->m_input = pts;
	for (const Point<Exact>& p : *pts) {
		VWPoint* vp = new VWPoint(p);
		this->m_complete.push_back(vp);
		this->m_current.push_back(vp);
	}
	for (int i = 0; i < this->m_current.size(); i++) {
		recomputeCost(i);
	}
}

VWSimplification::~VWSimplification() {
	for (VWPoint* vp : this->m_complete) {
		delete vp;
	}
}

Number<Exact> VWSimplification::constructAtComplexity(const int k) {
	continueToComplexity(k);

	this->m_input->clear();
	Number<Exact> maxCost = 0;

	for (VWPoint* vp : this->m_complete) {
		if (vp->removedAt <= k) {
			this->m_input->push_back(vp->pt);
		} else if (maxCost < vp->cost) {
			maxCost = vp->cost;
		}
	}

	return maxCost;
}

void VWSimplification::continueToComplexity(const int k) {
	while (this->m_current.size() > k) {
		int best = 0;
		for (int i = 1; i < this->m_current.size() - 1; i++) {
			if (best == 0 || this->m_current[best]->cost > this->m_current[i]->cost) {
				best = i;
			}
		}
		this->m_current[best]->removedAt = this->m_current.size();
		this->m_current.erase(this->m_current.begin() + best);
		recomputeCost(best - 1);
		recomputeCost(best);
	}
}

void VWSimplification::recomputeCost(const int i) {
	if (i == 0 || i == this->m_current.size() - 1) {
		this->m_current[i]->cost = -1;
	} else {
		this->m_current[i]->cost = CGAL::abs(CGAL::area(
		    this->m_current[i - 1]->pt, this->m_current[i]->pt, this->m_current[i + 1]->pt));
	}
}

VWPoint::VWPoint(const Point<Exact>& pt) : pt(pt), removedAt(-1), cost(0) {}

} // namespace cartocrow::vw_simplification
