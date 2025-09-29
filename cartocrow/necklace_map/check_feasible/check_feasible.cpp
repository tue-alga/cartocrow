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

#include "check_feasible.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "check_feasible_exact.h"
#include "check_feasible_heuristic.h"

namespace cartocrow::necklace_map {
namespace detail {

CheckFeasible::Ptr CheckFeasible::construct(NodeSet& nodes, const int heuristic_cycles) {
	if (heuristic_cycles == 0) {
		return std::make_shared<CheckFeasibleExact>(nodes);
	} else {
		return std::make_shared<CheckFeasibleHeuristic>(nodes, heuristic_cycles);
	}
}

void CheckFeasible::Initialize() {
	InitializeSlices();
	InitializeContainer();
}

CheckFeasible::Value::Value() {
	Reset();
}

void CheckFeasible::Value::Reset() {
	task.reset();
	angle_rad = std::numeric_limits<Number<Inexact>>::max();
}

Number<Inexact> CheckFeasible::Value::CoveringRadius() const {
	if (!task || !task->bead) {
		return 0;
	}
	return task->bead->covering_radius_rad;
}

CheckFeasible::CheckFeasible(NodeSet& nodes) : nodes_(nodes), slices_() {}

void CheckFeasible::InitializeSlices() {
	// Construct a sorted list of events signifying where intervals begin and end.
	std::vector<TaskEvent> events;
	events.reserve(2 * nodes_.size());

	int max_layer = 0;
	for (const CycleNodeLayered::Ptr& node : nodes_) {
		events.emplace_back(node, wrapAngle(node->valid->from()), TaskEvent::Type::kFrom);
		events.emplace_back(node, wrapAngle(node->valid->to()), TaskEvent::Type::kTo);
		max_layer = std::max(max_layer, node->layer);
	}
	std::sort(events.begin(), events.end(), CompareTaskEvent());
	const int num_layers = max_layer + 1;

	// Construct the task slices.
	// Each task slice stores the nodes that are valid between two consecutive events, together with a bit string for each combination of these nodes.
	// For this purpose, keep track of the nodes that are valid at some angle, starting at 0 radians (because the first event has the smallest positive angle).
	std::vector<CycleNodeLayered::Ptr> active_nodes(num_layers);
	for (const CycleNodeLayered::Ptr& node : nodes_) {
		if (0 < node->valid->from() && M_2xPI <= node->valid->to()) {
			active_nodes[node->layer] = node;
		}
	}

	slices_.reserve(events.size());
	for (size_t i = 0; i < events.size(); ++i) {
		const TaskEvent& event_from = events[i];
		const TaskEvent& event_to = events[(i + 1) % events.size()];

		// Update the active nodes.
		active_nodes[event_from.node->layer] =
		    event_from.type == TaskEvent::Type::kFrom ? event_from.node : nullptr;

		// Construct a new slice.
		slices_.emplace_back(event_from, event_to, num_layers);
		for (const CycleNodeLayered::Ptr& node : active_nodes) {
			if (node) {
				slices_.back().AddTask(node);
			}
		}
		slices_.back().Finalize();
	}

	// Make sure that the first slice starts from a interval begin event.
	std::vector<TaskSlice>::iterator first_iter =
	    std::find_if(slices_.begin(), slices_.end(), [](const TaskSlice& slice) -> bool {
		    return slice.event_from.type == TaskEvent::Type::kFrom;
	    });
	if (first_iter != slices_.begin()) {
		std::rotate(slices_.begin(), first_iter, slices_.end());
	}
}

void CheckFeasible::InitializeContainer() {
	// Construct the dynamic programming results container.
	const int num_subsets = std::pow(2, slices_.front().tasks.size());
	values_.resize(slices_.size());
	for (std::vector<Value>& subsets : values_) {
		subsets.resize(num_subsets);
	}
}

void CheckFeasible::ResetContainer() {
	if (values_[0][0].angle_rad == std::numeric_limits<Number<Inexact>>::max()) {
		return;
	}

	// Reset the dynamic programming results container.
	for (std::vector<Value>& subsets : values_) {
		for (Value& value : subsets) {
			value.Reset();
		}
	}
}

void CheckFeasible::FillContainer(const size_t first_slice_index,
                                  const BitString& first_slice_layer_set,
                                  const BitString& first_slice_remaining_set) {
	// Initialize the values.
	values_[0][0].task = std::make_shared<CycleNodeLayered>();
	values_[0][0].angle_rad = 0;

	const size_t num_slices = slices_.size();
	for (size_t value_index = 0; value_index < num_slices; ++value_index) {
		std::vector<Value>& slice_values = values_[value_index];

		const size_t slice_index = (value_index + first_slice_index) % num_slices;
		const TaskSlice& slice = slices_[slice_index];
		BitString slice_layer_string = BitString::fromBit(slice.event_from.node->layer);

		for (const BitString& layer_set : slice.layer_sets) {
			if (value_index == 0 && layer_set.isEmpty()) {
				continue;
			}

			Value& value = slice_values[layer_set.get()];
			value.task = nullptr;
			value.angle_rad = std::numeric_limits<Number<Inexact>>::max();

			if (value_index == 0 && layer_set.overlaps(first_slice_remaining_set)) {
				continue;
			}
			if (value_index == num_slices - 1 && layer_set.overlaps(first_slice_layer_set)) {
				continue;
			}

			if (0 < value_index) {
				// Check the previous slice.
				const std::vector<Value>& subset_prev = values_[value_index - 1];
				if (slice.event_from.type == TaskEvent::Type::kFrom) {
					if (!layer_set[slice.event_from.node->layer]) {
						value = subset_prev[layer_set.get()];
					}
				} else {
					const TaskSlice& slice_prev =
					    slices_[(slice_index + num_slices - 1) % num_slices];

					if (!slice_prev.tasks[slice.event_from.node->layer] ||
					    slice_prev.tasks[slice.event_from.node->layer]->disabled) { // Special case.
						value = subset_prev[layer_set.get()];
					} else {
						value = subset_prev[(layer_set + slice_layer_string).get()];
					}
				}
			}
			if (value.angle_rad < std::numeric_limits<Number<Inexact>>::max()) {
				continue;
			}

			for (const CycleNodeLayered::Ptr& task : slice.tasks) {
				if (!task || task->disabled || !layer_set[task->layer]) {
					continue;
				}

				const BitString layer_set_without_task = layer_set - BitString::fromBit(task->layer);
				const Value& value_without_task = slice_values[layer_set_without_task.get()];

				Number<Inexact> angle_rad = value_without_task.angle_rad;
				if (angle_rad == std::numeric_limits<Number<Inexact>>::max()) {
					continue;
				}

				if (value_without_task.task->bead) {
					angle_rad += value_without_task.task->bead->covering_radius_rad +
					             task->bead->covering_radius_rad;
				} else if (task->layer != slice.event_from.node->layer) {
					continue;
				}
				angle_rad = std::max(angle_rad, task->valid->from());

				// Check whether the task would still be in its valid interval.
				if (task->valid->to() < angle_rad) {
					continue;
				}

				// Check whether the counterclockwise extreme of the bead of this task is closer to the start than any others.
				if (value.angle_rad == std::numeric_limits<Number<Inexact>>::max() ||
				    angle_rad + task->bead->covering_radius_rad <
				        value.angle_rad + value.task->bead->covering_radius_rad) {
					value.task = task;
					value.angle_rad = angle_rad;
				}
			}
		}
	}
}

bool CheckFeasible::ProcessContainer(const size_t first_slice_index,
                                     const BitString& first_slice_remaining_set) {
	const TaskSlice& slice = slices_[first_slice_index];

	// Check whether the last slice was assigned a value.
	const size_t num_slices = slices_.size();
	const Value& value_last_unused = values_[num_slices - 1][first_slice_remaining_set.get()];
	if (value_last_unused.angle_rad == std::numeric_limits<Number<Inexact>>::max()) {
		return false;
	}

	// Assign an angle to each node.
	BitString layer_set = first_slice_remaining_set;
	Number<Inexact> check_angle_rad = std::numeric_limits<Number<Inexact>>::max();
	for (ptrdiff_t value_index = num_slices - 1;
	     0 <= value_index && values_[value_index][layer_set.get()].task &&
	     values_[value_index][layer_set.get()].task->layer != -1;) {
		Value& value = values_[value_index][layer_set.get()];
		const size_t value_slice_index = (value_index + first_slice_index) % num_slices;
		TaskSlice& value_slice = slices_[value_slice_index];

		const CycleNodeLayered::Ptr& task = value.task;
		if (value.angle_rad + M_EPSILON < value_slice.coverage.from() ||
		    (value.angle_rad < value_slice.coverage.from() + M_EPSILON &&
		     !(task && layer_set[task->layer]))) {
			// Move to the previous slice.
			const size_t prev_slice_index = (value_slice_index + num_slices - 1) % num_slices;
			const int& value_slice_layer = value_slice.event_from.node->layer;
			if (value_slice.event_from.type == TaskEvent::Type::kTo &&
			    slices_[prev_slice_index].tasks[value_slice_layer] &&
			    !slices_[prev_slice_index].tasks[value_slice_layer]->disabled) {
				layer_set += value_slice_layer;
			}

			--value_index;
		} else {
			if (!task || !layer_set[task->layer]) {
				return false;
			}

			assert(value.angle_rad <= check_angle_rad);
			check_angle_rad = value.angle_rad;

			// Assign the angle.
			AssignAngle(value.angle_rad + slice.event_from.angle_rad, task->bead);

			layer_set -= task->layer;
		}
	}

	return true;
}

} // namespace detail
} // namespace cartocrow::necklace_map
