/*
The Necklace Map library implements the algorithmic geo-visualization
method by the same name, developed by Bettina Speckmann and Kevin Verbeek
at TU Eindhoven (DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
Copyright (C) 2021  Netherlands eScience Center and TU Eindhoven

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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 06-05-2020
*/

#include "check_feasible_heuristic.h"

namespace cartocrow::necklace_map {
namespace detail {

CycleNodeCheck::CycleNodeCheck(const std::shared_ptr<Bead>& bead, const Number<Inexact>& angle_rad)
    : CycleNode(bead, std::make_shared<Range>(angle_rad - bead->covering_radius_rad,
                                              angle_rad + bead->covering_radius_rad)),
      check(0), angle_rad(angle_rad) {}

CheckFeasibleHeuristic::CheckFeasibleHeuristic(NodeSet& nodes, const int heuristic_cycles)
    : CheckFeasible(nodes), heuristic_cycles_(heuristic_cycles) {}

bool CheckFeasibleHeuristic::operator()() {
	if (slices_.empty()) {
		return true;
	}

	ResetContainer();

	return Feasible();
}

void CheckFeasibleHeuristic::InitializeSlices() {
	CheckFeasible::InitializeSlices();
	const size_t num_slices = slices_.size();

	// The main method in which the heuristic algorithm tries to save time is by stacking a number of duplicate slice collections back-to-back.
	// The solution is then decided in intervals of length 2pi on these slices.

	// Clone the slices.
	std::vector<TaskSlice> slices_clone;
	slices_clone.swap(slices_);

	slices_.reserve(num_slices * heuristic_cycles_);
	for (int cycle = 0; cycle < heuristic_cycles_; ++cycle) {
		for (size_t j = 0; j < num_slices; ++j) {
			slices_.emplace_back(slices_clone[j], slices_clone[0].coverage.from(), cycle);
		}
	}
}

void CheckFeasibleHeuristic::AssignAngle(const Number<Inexact>& angle_rad,
                                         std::shared_ptr<Bead>& bead) {
	assert(bead != nullptr);
	nodes_check_.push_back(std::make_shared<CycleNodeCheck>(bead, angle_rad));
}

bool CheckFeasibleHeuristic::Feasible() {
	FillContainer(0, BitString(), BitString());

	nodes_check_.clear();
	if (!ProcessContainer(0, slices_.back().layer_sets.back())) {
		return false;
	}

	// Check whether any nodes overlap.
	// Note that the nodes to check are in clockwise order.
	size_t count = 0;
	for (CheckSet::iterator left_iter = nodes_check_.begin(), right_iter = nodes_check_.begin();
	     left_iter != nodes_check_.end() && right_iter != nodes_check_.end();) {
		if ((*right_iter)->valid->from() + M_2xPI < (*left_iter)->valid->to()) {
			if (--(*right_iter)->check == 0) {
				--count;
			}
			++right_iter;
		} else {
			if (++(*left_iter)->check == 1) {
				if (++count == nodes_check_.size()) {
					// Feasible interval found; adjust the angles to use this interval.
					for (CheckSet::iterator node_iter = left_iter; node_iter != right_iter;
					     --node_iter) {
						std::shared_ptr<Bead>& bead = (*node_iter)->bead;
						bead->angle_rad = wrapAngle((*node_iter)->angle_rad);
					}
					return true;
				}
			}
			++left_iter;
		}
	}

	return false;
}

} // namespace detail
} // namespace cartocrow::necklace_map
