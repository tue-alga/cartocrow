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

#ifndef CARTOCROW_NECKLACE_MAP_DETAIL_TASK_H
#define CARTOCROW_NECKLACE_MAP_DETAIL_TASK_H

#include <vector>

#include "cartocrow/common/bit_string.h"
#include "cartocrow/common/core_types.h"
#include "cartocrow/common/range.h"
#include "cartocrow/necklace_map/detail/cycle_node_layered.h"

namespace cartocrow {
namespace necklace_map {
namespace detail {

// The beads will be processed by moving from event to event.
// Each event indicates that a valid interval starts or stops at the associated angle.
struct TaskEvent {
	enum class Type { kFrom, kTo };

	TaskEvent();

	TaskEvent(const CycleNodeLayered::Ptr& node, const Number& angle_rad, const Type& type);

	CycleNodeLayered::Ptr node;
	Number angle_rad;
	Type type;
}; // class TaskEvent

class CompareTaskEvent {
  public:
	bool operator()(const TaskEvent& a, const TaskEvent& b) const;
}; // class CompareTaskEvent

// A collection of tasks that are valid within some range.
// Note that within the complete range all these tasks are valid; they can only start and stop being valid at the start or end of the range.
class TaskSlice {
  public:
	TaskSlice();

	TaskSlice(const TaskEvent& event_from, const TaskEvent& event_to, const int num_layers);

	TaskSlice(const TaskSlice& slice, const Number& angle_start, const int cycle);

	void Reset();

	void Rotate(const TaskSlice& first_slice, const BitString& layer_set);

	void AddTask(const CycleNodeLayered::Ptr& task);

	void Finalize();

	TaskEvent event_from, event_to;
	Range coverage;

	std::vector<CycleNodeLayered::Ptr> tasks;
	std::vector<BitString> layer_sets;
}; // class TaskSlice

} // namespace detail
} // namespace necklace_map
} // namespace cartocrow

#endif //CARTOCROW_NECKLACE_MAP_DETAIL_TASK_H
