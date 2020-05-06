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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 02-04-2020
*/

#include "compute_scale_factor_any_order.h"

#include <algorithm>
#include <limits.h>

#include <glog/logging.h>


namespace geoviz
{
namespace necklace_map
{
namespace detail
{

// TODO(tvl) fix prototype code: const where possible
// TODO(tvl) go over code and add "override" or "final" keywords where applicable..

constexpr const int kMaxLayers = 15;

CycleNodeCheck::CycleNodeCheck(const Bead::Ptr& bead, const Range::Ptr& valid) :
  CycleNode(bead, valid), check(0)
{}





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
        } else
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


CheckFeasibleExact::CheckFeasibleExact(NodeSet& nodes) : CheckFeasible(nodes) {}

bool CheckFeasibleExact::operator()()
{
  if (slices_.empty())
    return true;

  ResetContainer();

  // Try each possibility that starts with an interval beginning event.
  for (size_t slice_index = 0; slice_index < slices_.size(); ++slice_index)
  {
    // The slice must start with an interval beginning event.
    const TaskSlice& slice = slices_[slice_index];
    if (slice.event_from.type == TaskEvent::Type::kTo)
      continue;

    for (const BitString& layer_set : slice.layer_sets)
    {
      // The layer set must include the beginning event's node.
      if (!layer_set[slice.event_from.node->layer])
        continue;

      // Split the circle at the starting event.
      SplitCircle(slice, layer_set);

      // If at least one set is feasible, the scale factor is feasible.
      if (FeasibleFromSlice(slice_index, layer_set))
        return true;
    }
  }

  return false;
}

void CheckFeasibleExact::SplitCircle
(
  const TaskSlice& first_slice,
  const BitString& layer_set
)
{
  // Reset each slice and then align it with the start of the current slice.
  for (TaskSlice& slice : slices_)
  {
    slice.Reset();
    slice.Rotate(first_slice, layer_set);
  }
}

bool CheckFeasibleExact::FeasibleFromSlice
(
  const size_t first_slice_index,
  const BitString& first_layer_set
)
{
  // Determine the layers of the slice that are not used.
  const TaskSlice& slice = slices_[first_slice_index];
  const BitString first_unused_set = first_layer_set ^(slice.layer_sets.back());

  ComputeValues(first_slice_index, first_layer_set, first_unused_set);
  return AssignAngles(first_slice_index, first_unused_set);
}


CheckFeasibleHeuristic::CheckFeasibleHeuristic(NodeSet& nodes, const int heuristic_cycles) :
  CheckFeasible(nodes),
  heuristic_cycles_(heuristic_cycles) {}

bool CheckFeasibleHeuristic::operator()()
{
  if (slices_.empty())
    return true;

  ResetContainer();

  return Feasible();
}

void CheckFeasibleHeuristic::InitializeSlices()
{
  CheckFeasible::InitializeSlices();

  // The main method in which the heuristic algorithm tries to save time is by stacking a number of duplicate slice collections back-to-back.
  // The solution is then decided in intervals of length 2pi on these slices.

  // Clone the slices.
  const size_t num_slices = slices_.size();
  slices_.resize(num_slices * heuristic_cycles_);
  for (int cycle = 1; cycle < heuristic_cycles_; ++cycle)
    for (size_t j = 0; j < num_slices; ++j)
      slices_[cycle * num_slices + j] = TaskSlice(slices_[j], cycle);
}

void CheckFeasibleHeuristic::ProcessTask(const CycleNodeLayered::Ptr& task)
{
  const Number covering_radius_rad = task->bead ? task->bead->covering_radius_rad : 0;
  nodes_check_.push_back
  (
    std::make_shared<CycleNodeCheck>
    (
      task->bead,
      std::make_shared<Range>
      (
        task->bead->angle_rad - covering_radius_rad,
        task->bead->angle_rad + covering_radius_rad
      )
    )
  );
}

bool CheckFeasibleHeuristic::Feasible()
{
  ComputeValues(0, BitString(), BitString());

  nodes_check_.clear();
  if (!AssignAngles(0, slices_.back().layer_sets.back()))
    return false;

  // Check whether any nodes overlap.
  int count = 0;
  for
  (
    CheckSet::iterator left_iter = nodes_check_.begin(), right_iter = nodes_check_.begin();
    left_iter != nodes_check_.end() && right_iter != nodes_check_.end();
  )
  {
    if ((*right_iter)->valid->from() + M_2xPI < (*left_iter)->valid->to())
    {
      if (--(*right_iter)->check == 0)
        --count;
      ++right_iter;
    }
    else
    {
      if (++(*left_iter)->check == 1)
        if (++count == nodes_check_.size())
          return true;
      ++left_iter;
    }
  }

  return false;
}


ComputeScaleFactorAnyOrder::ComputeScaleFactorAnyOrder
(
  const Necklace::Ptr& necklace,
  const Number& buffer_rad /*= 0*/,
  const int binary_search_depth /*= 10*/,
  const int heuristic_cycles /*= 5*/
) :
  necklace_shape_(necklace->shape),
  half_buffer_rad_(0.5 * buffer_rad),
  max_buffer_rad_(0),
  binary_search_depth_(binary_search_depth)
{
  // Collect and order the beads based on the start of their valid interval (initialized as their feasible interval).
  for (const Bead::Ptr& bead : necklace->beads)
    nodes_.push_back(std::make_shared<CycleNodeLayered>(bead));

  std::sort(nodes_.begin(), nodes_.end(), CompareCycleNodeLayered());

  // Prepare the feasibility check.
  check_ = CheckFeasible::New(nodes_, heuristic_cycles);
}

Number ComputeScaleFactorAnyOrder::Optimize()
{
  // Assign a layer to each node such that the nodes in a layer do not overlap in their feasibile intervals.
  const int num_layers = AssignLayers();

  // The algorithm is exponential in the number of layers, so we limit this number.
  if (kMaxLayers < num_layers)
    return 0;

  // Initialize the collection of task slices: collections of fixed tasks that are relevant within some angle range.
  check_->Initialize();

  // Perform a binary search on the scale factor, determining which are feasible.
  // This binary search requires a decent initial upper bound on the scale factor.
  Number lower_bound = 0;
  Number upper_bound = ComputeScaleUpperBound();

  for (int step = 0; step < binary_search_depth_; ++step)
  {
    double scale_factor = 0.5 * (lower_bound + upper_bound);
    ComputeCoveringRadii(scale_factor);

    if ((*check_)())
      lower_bound = scale_factor;
    else
      upper_bound = scale_factor;
  }

  ComputeBufferUpperBound(lower_bound);

  // The lower bound is the largest confirmed scale factor for which all beads could fit.
  return lower_bound;
}

Number ComputeScaleFactorAnyOrder::ComputeScaleUpperBound()
{
  // The initial upper bound make sure all beads would fit if they were the size of the smallest bead.
  Number upper_bound = 0;
  max_buffer_rad_ = 0;
  for (const CycleNodeLayered::Ptr& node : nodes_)
  {
    const Number radius_rad = necklace_shape_->ComputeCoveringRadiusRad(node->valid, node->bead->radius_base);
    upper_bound = std::max(upper_bound, M_PI / (radius_rad + half_buffer_rad_));

    // The maximum buffer will be based on the minimum radius and the final scale factor.
    if (0 < radius_rad)
      max_buffer_rad_ = std::min(max_buffer_rad_, radius_rad);
  }

  // Perform a binary search to find the largest scale factor for which all beads could fit.
  Number lower_bound = 0.0;
  for (int step = 0; step < binary_search_depth_; ++step)
  {
    const Number scale_factor = 0.5 * (lower_bound + upper_bound);

    Number totalSize = 0.0;
    for (const CycleNodeLayered::Ptr& node : nodes_)
      totalSize +=
        necklace_shape_->ComputeCoveringRadiusRad(node->valid, scale_factor * node->bead->radius_base) +
        half_buffer_rad_;

    // Check whether the scaled beads could fit.
    if (totalSize <= M_PI)
      lower_bound = scale_factor;
    else
      upper_bound = scale_factor;
  }

  // The lower bound is the largest confirmed scale factor for which all beads could fit.
  return lower_bound;
}

Number ComputeScaleFactorAnyOrder::ComputeCoveringRadii(const Number& scale_factor)
{
  for (CycleNodeLayered::Ptr& node : nodes_)
    node->bead->covering_radius_rad =
      necklace_shape_->ComputeCoveringRadiusRad(node->valid, node->bead->radius_base * scale_factor) + half_buffer_rad_;
}

int ComputeScaleFactorAnyOrder::AssignLayers()
{
  // Each node should be assigned a layer such that each layer does not contain any pair of nodes that overlap in their valid interval.
  // Note that this can be done greedily: assign the nodes by minimizing the distance between the last valid interval and the next.

  using NodeList = std::list<CycleNodeLayered::Ptr>;
  NodeList remaining_nodes(nodes_.begin(), nodes_.end());

  int layer = 0;
  remaining_nodes.front()->layer = layer;
  NecklaceInterval layer_interval(*remaining_nodes.front()->valid);

  remaining_nodes.pop_front();
  NodeList::iterator node_iter = remaining_nodes.begin();
  NodeList::iterator unused_iter = remaining_nodes.end();

  // Note that the nodes are already ordered by the starting angle of their valid interval.
  while (!remaining_nodes.empty())
  {
    if (!layer_interval.IntersectsOpen((*node_iter)->valid))
    {
      // Add the non-overlapping node to the layer.
      (*node_iter)->layer = layer;
      layer_interval.to_rad() = ModuloNonZero((*node_iter)->valid->to(), layer_interval.from_rad());
      node_iter = remaining_nodes.erase(node_iter);
    } else if (node_iter == unused_iter)
    {
      // All nodes were checked: start a new layer.
      ++layer;
      (*node_iter)->layer = layer;
      layer_interval = NecklaceInterval(*(*node_iter)->valid);

      node_iter = remaining_nodes.erase(node_iter);
      unused_iter = remaining_nodes.end();
    } else
    {
      if (unused_iter == remaining_nodes.end())
        // Mark the node as the first one of the next layer.
        unused_iter = node_iter;
      ++node_iter;
    }

    if (node_iter == remaining_nodes.end())
      node_iter = remaining_nodes.begin();
  }

  return layer + 1;
}

void ComputeScaleFactorAnyOrder::ComputeBufferUpperBound(const Number& scale_factor)
{
  max_buffer_rad_ *= scale_factor;
}


ComputeScaleFactorAnyOrderIngot::ComputeScaleFactorAnyOrderIngot
(
  const Necklace::Ptr& necklace,
  const Number& buffer_rad /*= 0*/,
  const int binary_search_depth /*= 10*/,
  const int heuristic_cycles /*= 5*/
) :
  ComputeScaleFactorAnyOrder(necklace, buffer_rad, binary_search_depth, heuristic_cycles)
{}

Number ComputeScaleFactorAnyOrderIngot::ComputeScaleUpperBound()
{
  max_buffer_rad_ = 0;
  for (const CycleNodeLayered::Ptr& node : nodes_)
  {
    const Number radius_rad = necklace_shape_->ComputeCoveringRadiusRad(node->valid, node->bead->radius_base);

    // The maximum buffer will be based on the minimum radius and the final scale factor.
    if (0 < radius_rad)
      max_buffer_rad_ = std::min(max_buffer_rad_, radius_rad);
  }

  return M_PI / nodes_.size() - half_buffer_rad_;
}

Number ComputeScaleFactorAnyOrderIngot::ComputeCoveringRadii(const Number& scale_factor)
{
  for (CycleNodeLayered::Ptr& node : nodes_)
    node->bead->covering_radius_rad = scale_factor + half_buffer_rad_;
}

} // namespace detail
} // namespace necklace_map
} // namespace geoviz
