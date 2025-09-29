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

#include "task.h"
#include "cartocrow/core/core.h"

#include <algorithm>
#include <cmath>

namespace cartocrow::necklace_map {
namespace detail {

TaskEvent::TaskEvent() {}

TaskEvent::TaskEvent(const CycleNodeLayered::Ptr& node, const Number<Inexact>& angle_rad,
                     const Type& type)
    : node(node), angle_rad(angle_rad), type(type) {}

bool CompareTaskEvent::operator()(const TaskEvent& a, const TaskEvent& b) const {
	if (a.angle_rad != b.angle_rad) {
		return a.angle_rad < b.angle_rad;
	}

	// Note that we should return false whenever a is not 'smaller' than b.
	// Practically, 'end' events should be handled before 'start' events.
	// There is a specific exception: when the events have the same node,
	// the 'start' event must be handled before the 'end' event (this is a degenerate interval).
	if (a.node == b.node) {
		return a.type == TaskEvent::Type::kFrom;
	}

	return a.type == TaskEvent::Type::kTo && b.type == TaskEvent::Type::kFrom;
}

TaskSlice::TaskSlice() : event_from(), event_to(), coverage(0, 0), tasks(), layer_sets() {}

TaskSlice::TaskSlice(const TaskEvent& event_from, const TaskEvent& event_to, const int num_layers)
    : event_from(event_from), event_to(event_to),
      coverage(event_from.angle_rad, wrapAngle(event_to.angle_rad, event_from.angle_rad)) {
	assert(BitString::checkFit(num_layers));
	tasks.resize(num_layers);
}

TaskSlice::TaskSlice(const TaskSlice& slice, const Number<Inexact>& angle_start, const int cycle)
    : event_from(slice.event_from), event_to(slice.event_to), coverage(0, 0),
      layer_sets(slice.layer_sets) {
	assert(0 <= cycle);

	// Determine the part of the necklace covered by this slice.
	const Number<Inexact> cycle_start = cycle * M_2xPI;
	const Number<Inexact> offset = cycle_start - angle_start;
	coverage.from() = wrapAngle(event_from.angle_rad + offset, cycle_start);
	coverage.to() = wrapAngle(event_to.angle_rad + offset, coverage.from());

	// Copy the tasks.
	tasks.reserve(slice.tasks.size());
	for (const CycleNodeLayered::Ptr& task : slice.tasks) {
		tasks.emplace_back();
		if (!task) {
			continue;
		}

		// Skip tasks that have started before the first cycle.
		if (cycle == 0 && coverage.to() <= task->valid->from() + offset &&
		    task->valid->contains(M_2xPI + angle_start)) {
			continue;
		}

		// Note that we must clone the task, i.e. construct a separate object, with its valid range offset to fit the slice.
		tasks.back() = std::make_shared<CycleNodeLayered>(*task);
		tasks.back()->valid =
		    std::make_shared<Range>(task->valid->from() + offset, task->valid->to() + offset);
	}

	Finalize();
}

void TaskSlice::Reset() {
	coverage = CircularRange(event_from.angle_rad, event_to.angle_rad);
	for (const CycleNodeLayered::Ptr& task : tasks) {
		if (!task) {
			continue;
		}

		task->valid = std::make_shared<CircularRange>(task->bead->feasible);
		task->disabled = false;
	}
}

void TaskSlice::Rotate(const TaskSlice& first_slice, const BitString& layer_set) {
	// Rotate this slice such that the origin is aligned with the start of the other slice.
	const Number<Inexact>& angle_rad = first_slice.event_from.angle_rad;
	coverage = CircularRange(coverage.from() - angle_rad, coverage.to() - angle_rad);
	if (coverage.to() < M_EPSILON) { // TODO why are we using epsilon here? (and later)
		coverage.to() = M_2xPI;
	}

	for (const CycleNodeLayered::Ptr& task : tasks) {
		if (!task) {
			continue;
		}

		task->valid = std::make_shared<CircularRange>(task->valid->from() - angle_rad,
		                                              task->valid->to() - angle_rad);

		if (first_slice.tasks[task->layer] && first_slice.tasks[task->layer]->bead == task->bead) {
			// Disable tasks that start before the first slice, except when that task's bead caused the event to start the first slice.
			if (layer_set[task->layer]) {
				if (task->bead != first_slice.event_from.node->bead &&
				    task->valid->to() <= coverage.from() + M_EPSILON) {
					task->disabled = true;
				}
				task->valid->from() = 0;
			} else {
				if (coverage.to() - M_EPSILON <= task->valid->from()) {
					task->disabled = true;
				}
				task->valid->to() = M_2xPI;
			}
		} else {
			if (task->valid->to() < M_EPSILON) {
				task->valid->to() = M_2xPI;
			}
		}
	}
}

void TaskSlice::AddTask(const CycleNodeLayered::Ptr& task) {
	assert(task->layer < tasks.size());
	tasks[task->layer] = std::make_shared<CycleNodeLayered>(task);
}

void TaskSlice::Finalize() {
	// The layer sets are all permutations of used layers described as bit strings.
	layer_sets.clear();
	layer_sets.reserve(std::pow(2, tasks.size()));
	layer_sets.emplace_back();
	for (const CycleNodeLayered::Ptr& task : tasks) {
		if (!task) {
			continue;
		}

		std::transform(layer_sets.begin(), layer_sets.end(), std::back_inserter(layer_sets),
		               [task](const BitString& string) -> BitString {
			               return string + BitString::fromBit(task->layer);
		               });
	}
}

} // namespace detail
} // namespace cartocrow::necklace_map
