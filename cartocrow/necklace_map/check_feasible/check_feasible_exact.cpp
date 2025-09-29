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

#include "check_feasible_exact.h"

namespace cartocrow::necklace_map {
namespace detail {

CheckFeasibleExact::CheckFeasibleExact(NodeSet& nodes) : CheckFeasible(nodes) {}

bool CheckFeasibleExact::operator()() {
	if (slices_.empty()) {
		return true;
	}

	ResetContainer();

	// Try each possibility that starts with an interval beginning event.
	for (size_t slice_index = 0; slice_index < slices_.size(); ++slice_index) {
		// The slice must start with an interval beginning event.
		const TaskSlice& slice = slices_[slice_index];
		if (slice.event_from.type == TaskEvent::Type::kTo) {
			continue;
		}

		for (const BitString& layer_set : slice.layer_sets) {
			// The layer set must include the beginning event's node.
			if (!layer_set[slice.event_from.node->layer]) {
				continue;
			}

			// Split the circle at the starting event.
			SplitCircle(slice, layer_set);

			// If at least one set is feasible, the scale factor is feasible.
			if (FeasibleFromSlice(slice_index, layer_set)) {
				return true;
			}
		}
	}

	return false;
}

void CheckFeasibleExact::SplitCircle(const TaskSlice& first_slice, const BitString& layer_set) {
	// Reset each slice and then align it with the start of the current slice.
	for (TaskSlice& slice : slices_) {
		slice.Reset();
	}
	for (TaskSlice& slice : slices_) {
		slice.Rotate(first_slice, layer_set);
	}
}

bool CheckFeasibleExact::FeasibleFromSlice(const size_t first_slice_index,
                                           const BitString& first_slice_layer_set) {
	// Determine the layers of the slice that are not used.
	const TaskSlice& slice = slices_[first_slice_index];
	const BitString first_slice_others_set = first_slice_layer_set ^ (slice.layer_sets.back());

	FillContainer(first_slice_index, first_slice_layer_set, first_slice_others_set);

	// Check whether the last slice was assigned a value.
	const size_t num_slices = slices_.size();
	const Value& value_last_unused = values_[num_slices - 1][first_slice_others_set.get()];
	if (value_last_unused.angle_rad == std::numeric_limits<Number<Inexact>>::max()) {
		return false;
	}

	// Check whether the first and last beads overlap.
	if (M_2xPI < value_last_unused.angle_rad + value_last_unused.task->bead->covering_radius_rad +
	                 slice.event_from.node->bead->covering_radius_rad) {
		return false;
	}

	bead_angles_.clear();
	if (!ProcessContainer(first_slice_index, first_slice_others_set)) {
		return false;
	}

	for (BeadAngleMap::iterator iter = bead_angles_.begin(); iter != bead_angles_.end(); ++iter) {
		iter->second->angle_rad = wrapAngle(iter->first);
	}
	return true;
}

void CheckFeasibleExact::AssignAngle(const Number<Inexact>& angle_rad, std::shared_ptr<Bead>& bead) {
	assert(bead != nullptr);
	bead_angles_[angle_rad] = bead;
}

} // namespace detail
} // namespace cartocrow::necklace_map
