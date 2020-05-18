/*
The Necklace Map library implements the algorithmic geo-visualization
method by the same name, developed by Bettina Speckmann and Kevin Verbeek
at TU Eindhoven (DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
Copyright (C) 2019  Netherlands eScience Center and TU Eindhoven

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

#include "geoviz/necklace_map/detail/check_feasible_exact.h"
#include "geoviz/necklace_map/detail/check_feasible_heuristic.h"


namespace geoviz
{
namespace necklace_map
{
namespace detail
{

CheckFeasible::Ptr CheckFeasible::New(NodeSet& nodes, const int heuristic_cycles)
{
  if (heuristic_cycles == 0)
    return std::make_shared<CheckFeasibleExact>(nodes);
  else
    return std::make_shared<CheckFeasibleHeuristic>(nodes, heuristic_cycles);
}

void CheckFeasible::Initialize()
{
  InitializeSlices();
  InitializeContainer();
}

CheckFeasible::Value::Value()
{
  Reset();
}

void CheckFeasible::Value::Reset()
{
  task.reset();
  angle_rad = std::numeric_limits<Number>::max();
}

Number CheckFeasible::Value::CoveringRadius() const
{
  if (!task || !task->bead)
    return 0;
  return task->bead->covering_radius_rad;
}

CheckFeasible::CheckFeasible(NodeSet& nodes) : slices_(), nodes_(nodes) {}

void CheckFeasible::InitializeSlices()
{
  // Construct a sorted list of events signifying where intervals begin and end.
  std::vector<TaskEvent> events;
  events.reserve(2 * nodes_.size());

  int max_layer = 0;
  for (const CycleNodeLayered::Ptr& node : nodes_)
  {
    events.emplace_back(node, node->valid->from(), TaskEvent::Type::kFrom);
    events.emplace_back(node, node->valid->to(), TaskEvent::Type::kTo);
    max_layer = std::max(max_layer, node->layer);
  }
  std::sort(events.begin(), events.end(), CompareTaskEvent());
  const int num_layers = max_layer + 1;

  // Construct the task slices.
  // Each task slice stores the nodes that are valid between two consecutive events, together with a bit string for each combination of these nodes.
  // For this purpose, keep track of the nodes that are valid at some angle, starting at 0 radians (because the first event has the smallest positive angle).
  CycleNodeLayered::Ptr active_nodes[num_layers];
  for (const CycleNodeLayered::Ptr& node : nodes_)
    if (node->valid->Contains(0))  // Note that this purposely excludes nodes that contain 2pi.
      active_nodes[node->layer] = node;

  slices_.reserve(events.size());
  for (int i = 0; i < events.size(); ++i)
  {
    const TaskEvent& event_from = events[i];
    const TaskEvent& event_to = events[(i + 1) % events.size()];

    // Update the active nodes.
    active_nodes[event_from.node->layer] = event_from.type == TaskEvent::Type::kFrom ? event_from.node : nullptr;

    // Construct a new slice.
    slices_.emplace_back(event_from, event_to, num_layers);
    for (const CycleNodeLayered::Ptr& node : active_nodes)
      if (node)
        slices_.back().AddTask(node);
    slices_.back().Finalize();
  }

  // Note that the first slice must start from a interval begin event.
  // Due to the valid intervals being Ranges with strictly non-negative angles, this must be true by construction.
//  std::vector<TaskSlice>::iterator first_iter =
//    std::find_if
//    (
//      slices_.begin(),
//      slices_.end(),
//      [](const TaskSlice& slice) -> bool { return slice.event_from.type == TaskEvent::Type::kFrom; }
//    );
//  if (first_iter != slices_.begin())
//    std::rotate(slices_.begin(), first_iter, slices_.end());
}

void CheckFeasible::InitializeContainer()
{
  // Construct the dynamic programming results container.
  const int num_subsets = std::pow(2, slices_.front().tasks.size());
  values_.resize(slices_.size());
  for (std::vector<Value>& subsets : values_)
    subsets.resize(num_subsets);
}

void CheckFeasible::ResetContainer()
{
  if (values_[0][0].angle_rad == std::numeric_limits<double>::max())
    return;

  // Reset the dynamic programming results container.
  for (std::vector<Value>& subsets : values_)
    for (Value& value : subsets)
      value.Reset();
}

void CheckFeasible::ComputeValues
(
  const size_t slice_index_offset,
  const BitString& first_layer_set,
  const BitString& first_unused_set
)
{
  // Initialize the values.
  values_[0][0].task = std::make_shared<CycleNodeLayered>();
  values_[0][0].angle_rad = 0;

  const size_t num_slices = slices_.size();
  for (size_t value_index = 0; value_index < num_slices; ++value_index)
  {
    std::vector<Value>& subset = values_[value_index];

    const size_t slice_index = (value_index + slice_index_offset) % num_slices;
    const TaskSlice& slice = slices_[slice_index];
    BitString slice_layer_string = BitString::FromBit(slice.event_from.node->layer);

    for (const BitString& layer_set : slice.layer_sets)
    {
      if (value_index == 0 && layer_set.IsEmpty())
        continue;

      Value& value = subset[layer_set.Get()];
      value.task = nullptr;
      value.angle_rad = std::numeric_limits<double>::max();

      if (value_index == 0 && first_unused_set.Overlaps(layer_set))
        continue;
      if (value_index == num_slices - 1 && first_layer_set.Overlaps(layer_set))
        continue;

      if (0 < value_index)
      {
        // Check the previous slice.
        const std::vector<Value>& subset_prev = values_[value_index - 1];
        if (slice.event_from.type == TaskEvent::Type::kFrom)
        {
          if (!layer_set[slice.event_from.node->layer])
            value = subset_prev[layer_set.Get()];
        }
        else
        {
          const TaskSlice& slice_prev = slices_[(slice_index + num_slices - 1) % num_slices];

          if
          (
            !slice_prev.tasks[slice.event_from.node->layer] ||
            slice_prev.tasks[slice.event_from.node->layer]->disabled
          )  // Special case.
            value = subset_prev[layer_set.Get()];
          else
            value = subset_prev[(layer_set + slice_layer_string).Get()];
        }
      }
      if (value.angle_rad < std::numeric_limits<double>::max())
        continue;

      for (const CycleNodeLayered::Ptr& task : slice.tasks)
      {
        if (!task || task->disabled || !layer_set[task->layer])
          continue;

        const Value& value_without_task = subset[(layer_set - BitString::FromBit(task->layer)).Get()];

        Number angle_rad = value_without_task.angle_rad;
        if (angle_rad == std::numeric_limits<double>::max())
          continue;

        if (value_without_task.task->bead)
          angle_rad += value_without_task.task->bead->covering_radius_rad + task->bead->covering_radius_rad;
//        else if (task->layer != slice.event_from.node->layer)
//          continue;
        angle_rad = std::max(angle_rad, task->valid->from());

        // Check whether the tasks would still be in the valid interval.
        if (task->valid->to() < angle_rad)
          continue;

        // Check whether this task is closer than any others.
        if
        (
          value.angle_rad == std::numeric_limits<double>::max() ||
          angle_rad + task->bead->covering_radius_rad < value.angle_rad + value.task->bead->covering_radius_rad
        )
        {
          value.task = task;
          value.angle_rad = angle_rad;
        }
      }
    }
  }
}

bool CheckFeasible::AssignAngles
(
  const size_t slice_index_offset,
  const BitString& first_unused_set
)
{
  // Check whether the last slice was assigned a value.
  const TaskSlice& slice = slices_[slice_index_offset];

  const size_t num_slices = slices_.size();
  const Value& value_last_unused = values_[num_slices - 1][first_unused_set.Get()];
  if (value_last_unused.angle_rad == std::numeric_limits<double>::max())
    return false;

  // Assign an angle to each node.
  BitString layer_set = first_unused_set;

  for
  (
    ptrdiff_t value_index = num_slices - 1;
    0 <= value_index && values_[value_index][layer_set.Get()].task->layer != -1;
  )
  {
    Value& value = values_[value_index][layer_set.Get()];
    const size_t value_slice_index = (value_index + slice_index_offset) % num_slices;
    TaskSlice& value_slice = slices_[value_slice_index];

    if (value.angle_rad + kEpsilon < value_slice.coverage.from())
    {
      // Move to the previous slice.
      const int& value_slice_layer = value_slice.event_from.node->layer;
      if
      (
        value_slice.event_from.type == TaskEvent::Type::kTo &&
        slices_[(value_slice_index + num_slices - 1) % num_slices].tasks[value_slice_layer] &&
        !slices_[(value_slice_index + num_slices - 1) % num_slices].tasks[value_slice_layer]->disabled
      )
        layer_set += value_slice_layer;

      --value_index;
    } else
    {
      const CycleNodeLayered::Ptr& task = value.task;
      if (!task || !layer_set[task->layer])
        return false;

      layer_set -= task->layer;

      // Assign the angle.
      task->bead->angle_rad = value.angle_rad + slice.event_from.angle_rad;

      ProcessTask(task);
    }
  }

  return true;
}

} // namespace detail
} // namespace necklace_map
} // namespace geoviz