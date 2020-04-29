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

// TODO(tvl) Note, directly based on the Java implementation: clean up.

// TODO(tvl) fix prototype code: const where possible
// TODO(tvl) fix prototype code: collection loops
// TODO(tvl) fix prototype code: variable names
// TODO(tvl) fix prototype code: method names (CamelCase)
// TODO(tvl) fix prototype code: comments.
// TODO(tvl) fix prototype code: reduce looping behaviour.
// TODO(tvl) go over code and add "override" or "final" keywords where applicable..

constexpr const Number kEpsilon = 0.0000001;
constexpr const int kMaxLayers = 15;


AnyOrderCycleNode::AnyOrderCycleNode() :
  CycleNode(), layer(-1), disabled(true)
{}

AnyOrderCycleNode::AnyOrderCycleNode(const Bead::Ptr& bead) :
  CycleNode(bead), layer(-1), disabled(!bead)
{}

AnyOrderCycleNode::AnyOrderCycleNode(const AnyOrderCycleNode::Ptr& node) :
  CycleNode(), layer(-1), disabled(!node)
{
  if (node)
  {
    bead = node->bead;
    valid = node->valid;
    layer = node->layer;
  }
}

bool CompareAnyOrderCycleNode::operator()(const AnyOrderCycleNode::Ptr& a, const AnyOrderCycleNode::Ptr& b) const
{
  return a->valid->from() < b->valid->from();
}


TaskEvent::TaskEvent() {}

TaskEvent::TaskEvent(AnyOrderCycleNode::Ptr& node, const Number& angle_rad, const Type& type) :
  node(node),
  angle_rad(angle_rad),
  type(type)
{}

bool CompareTaskEvent::operator()(const TaskEvent& a, const TaskEvent& b) const
{
  if (a.angle_rad != b.angle_rad)
    return a.angle_rad < b.angle_rad;

  // Note that we should return false whenever a is not 'smaller' than b.
  // Practically, 'end' events should be handled before 'start' events.
  return a.type == TaskEvent::Type::kTo && b.type == TaskEvent::Type::kFrom;
}










bool BitString::CheckFit(const int bit) { return bit < 32; }

void BitString::SetBit(const int bit) { CHECK(CheckFit(bit)); bits = ToString(bit); }

bool BitString::AddBit(const int bit) { CHECK(CheckFit(bit)); return bits |= ToString(bit); }


TaskSlice::TaskSlice() :
  event_left(),
  event_right(),
  coverage(0, 0),
  tasks(),
  sets()
{}

TaskSlice::TaskSlice
(
  const TaskEvent& event_left,
  const TaskEvent& event_right,
  const int num_layers
) :
  event_left(event_left),
  event_right(event_right),
  coverage(event_left.angle_rad, event_right.angle_rad)
{
  CHECK(BitString::CheckFit(num_layers));
  tasks.resize(num_layers);
}

TaskSlice::TaskSlice(const TaskSlice& slice, const int cycle) :
  event_left(slice.event_left),
  event_right(slice.event_right),
  coverage(0, 0),
  sets(slice.sets)
{
  CHECK_LT(0, cycle);

  // Determine the part of the necklace covered by this slice, i.e. after circling the necklace a fixed number of times.
  const Number offset = cycle * M_2xPI;
  coverage.from() = event_left.angle_rad + offset;
  coverage.to() = Modulo(event_right.angle_rad + offset, coverage.from());

  // Copy the tasks.
  tasks.reserve(slice.tasks.size());
  for (const AnyOrderCycleNode::Ptr& task : slice.tasks)
  {
    tasks.emplace_back();
    if
    (
      task &&
      (
        coverage.to() <= task->valid->to() &&
        task->valid->from() != 0 &&
        task->valid->Contains(0)
      )
    )
    {
      // Note that we must clone the task, i.e. construct a separate object, with its valid range offset to fit the slice.
      tasks.back() = std::make_shared<AnyOrderCycleNode>(*task);
      tasks.back()->valid = std::make_shared<Range>
      (
        task->valid->from() + offset,
        task->valid->to() + offset
      );
    }
  }
}

void TaskSlice::Reset()
{
  coverage.from() = event_left.angle_rad;
  coverage.to() = event_right.angle_rad;
  for (const AnyOrderCycleNode::Ptr& task : tasks)
  {
    if (!task)
      continue;

    task->valid = std::make_shared<Range>(*task->bead->feasible);
    task->disabled = false;
  }
}

void TaskSlice::Rotate(const TaskSlice& slice, const BitString& layer_set)
{
  // Rotate this slice such that the origin is aligned with the start of the other slice.
  const Number& angle_rad = slice.event_left.angle_rad;
  coverage.from() -= angle_rad;
  coverage.to() -= angle_rad;
  if (coverage.to() < kEpsilon)
    coverage.to() = M_2xPI;

  for (const AnyOrderCycleNode::Ptr& task : tasks)
  {
    if (!task)
      continue;

    if (slice.tasks[task->layer] && slice.tasks[task->layer]->bead == task->bead)
    {
      if (layer_set.HasBit(task->layer))
      {
        task->valid->from() = 0.0;
        task->valid->to() -= angle_rad;
        if (task->valid->to() <= coverage.from() + kEpsilon)
          task->disabled = true;
      }
      else
      {
        task->valid->from() -= angle_rad;
        task->valid->to() = M_2xPI;
        if (coverage.to() <= task->valid->from() + kEpsilon)
          task->disabled = true;
      }
    }
    else
    {
      task->valid->from() -= angle_rad;
      task->valid->to() -= angle_rad;
      if (task->valid->to() < kEpsilon)
        task->valid->to() = M_2xPI;
    }
  }
}

void TaskSlice::AddTask(const AnyOrderCycleNode::Ptr& task)
{
  CHECK_LT(task->layer, tasks.size());
  tasks[task->layer] = task;
}

void TaskSlice::ConstructSets()
{
  // The sets are all permutations of used layers described as bit strings.
  sets.reserve(BitString::ToString(tasks.size()));
  sets.emplace_back(0);
  for (const AnyOrderCycleNode::Ptr& task : tasks)
  {
    if (!task)
      continue;

    std::transform
    (
      sets.begin(),
      sets.end(),
      std::back_inserter(sets),
      [task](BitString string) -> BitString { string.AddBit(task->layer); return string; }
    );
  }
}



CheckFeasible::Ptr CheckFeasible::New(NodeSet& nodes, const int heuristic_cycles)
{
  if (heuristic_cycles == 0)
    return std::make_shared<CheckFeasibleExact>(nodes);
  else
    return std::make_shared<CheckFeasibleHeuristic>(nodes, heuristic_cycles);
}

CheckFeasible::CheckFeasible(NodeSet& nodes): slices_(), nodes_(nodes) {}

void CheckFeasible::InitializeSlices()
{
  InitializeSlices_();
  InitializeContainer();
}

void CheckFeasible::InitializeContainer()
{


  // setup DP array
  values_.clear();
  values_.resize(slices_.size());

  const int num_layers = slices_.front().tasks.size();
  int nSubSets = (1 << num_layers);
  for (int i = 0; i < slices_.size(); i++)
    values_[i].resize(nSubSets);


}


CheckFeasible::Value::Value()
{
  Reset();
}

void CheckFeasible::Value::Reset()
{
  angle_rad = std::numeric_limits<Number>::max();
  angle2_rad = std::numeric_limits<Number>::max();
  layer = -1;
  task.reset();
}

void CheckFeasible::InitializeSlices_()
{




  // TODO(tvl) check which vectors can be replaced by simple fixed size arrays.
  std::vector<TaskEvent> events;
  events.reserve(2 * nodes_.size());

  int max_layer = 0;
  for (int i = 0; i < nodes_.size(); i++)
  {
    AnyOrderCycleNode::Ptr& ca = nodes_[i];
    max_layer = std::max(max_layer, ca->layer);

    events.emplace_back(ca, ca->valid->from(), TaskEvent::Type::kFrom);
    events.emplace_back(ca, ca->valid->to(), TaskEvent::Type::kTo);
  }
  std::sort(events.begin(), events.end(), CompareTaskEvent());  // TODO(tvl) is this even useful?

  const int num_layers = max_layer + 1;


  AnyOrderCycleNode::Ptr curNodes[num_layers];
  // initialize
  for (const AnyOrderCycleNode::Ptr& node : nodes_)
  {
    if (node->valid->Contains(0) && node->valid->to() < M_2xPI)
      curNodes[node->layer] = node;
  }



  //find taskslices
  slices_.resize(events.size());
  for (int i = 0; i < events.size(); i++)
  {
    TaskEvent e = events[i];
    TaskEvent e2 = events[(i + 1) % events.size()];
    slices_[i] = TaskSlice(e, e2, num_layers);
    if (e.type == TaskEvent::Type::kFrom)
      curNodes[e.node->layer] = e.node;
    else
      curNodes[e.node->layer] = nullptr;

    for (int j = 0; j < num_layers; j++)
      if (curNodes[j] != nullptr)
        slices_[i].AddTask(std::make_shared<AnyOrderCycleNode>(curNodes[j]));
  }

  for (TaskSlice& slice : slices_)
    slice.ConstructSets();

  // make sure first slice is start of task
  while (slices_[0].event_left.type == TaskEvent::Type::kTo)
  {
    const TaskSlice& slice = slices_[0];
    for (int i = 0; i < slices_.size() - 1; i++)
      slices_[i] = slices_[i + 1];
    slices_[slices_.size() - 1] = slice;
  }



}

void CheckFeasible::ResetContainer()
{
  if (values_[0][0].angle_rad == std::numeric_limits<double>::max())
    return;

  // reset DP array
  for (std::vector<Value>& values : values_)
    for (Value& value : values)
      value.Reset();



}


CheckFeasibleExact::CheckFeasibleExact(NodeSet& nodes) : CheckFeasible(nodes) {}

bool CheckFeasibleExact::operator()()
{
  if (slices_.empty())
    return true;

  ResetContainer();



  // try all possibilities
  for (int i = 0; i < slices_.size(); i++)
  {
    if (slices_[i].event_left.type == TaskEvent::Type::kFrom)
    {
      int q = (1 << slices_[i].event_left.node->layer);
      //int q = slices_[i].event_left.node->layer;
      for (int j = 0; j < slices_[i].sets.size(); j++)
      {
        const BitString& str2 = slices_[i].sets[j];
        int q2 = str2.Get();
        if ((q2&q) != 0) {

          // split the circle (ranges, event times, the works)
          SplitCircle(slices_[i], str2);

          // compute
          bool good = FeasibleLine(i, str2);

          if (good) return true;
        }
      }
    }
  }

  return false;
}

void CheckFeasibleExact::SplitCircle
(
  const TaskSlice& current_slice,
  const BitString& layer_set
)
{
  // Reset each slice and then align it with the start of the current slice.
  for (TaskSlice& slice : slices_)
  {
    slice.Reset();
    slice.Rotate(current_slice, layer_set);
  }
}

bool CheckFeasibleExact::FeasibleLine
(
  const int slice_index,
  const BitString& split
)
{
  BitString split2 = split.Xor(slices_[slice_index].sets.back());  //TODO(tvl) rename split_inverse?

  // initialization
  values_[0][0].angle_rad = 0.0;
  values_[0][0].layer = -1;
  values_[0][0].task = std::make_shared<AnyOrderCycleNode>();

  for (int i = 0; i < slices_.size(); i++)
  {
    int s = (slice_index + i) % slices_.size();
    const TaskSlice& slice = slices_[s];
    for (int j = 0; j < slice.sets.size(); j++)
    {
      const BitString& str = slice.sets[j];
      int q = str.Get();  // TODO(tvl) remove q.
      if (i == 0 && q == 0) continue;

      values_[i][q].angle_rad = std::numeric_limits<double>::max();
      values_[i][q].layer = -1;
      values_[i][q].task = nullptr;

      if (i == 0 && split2.HasAny(str)) continue;
      if (i == slices_.size() - 1 && split.HasAny(str)) continue;

      if (i != 0)
      {
        // check previous slice
        if (slice.event_left.type == TaskEvent::Type::kFrom)
        {
          if ((q & (1 << slice.event_left.node->layer)) == 0)
          {
            values_[i][q].angle_rad = values_[i - 1][q].angle_rad;
            values_[i][q].layer = values_[i - 1][q].layer;
            values_[i][q].task = values_[i - 1][q].task;
          }
        } else
        {
          int q2 = q + (1 << slice.event_left.node->layer);
          if (slices_[(s + slices_.size() - 1) % slices_.size()].tasks[slice.event_left.node->layer]->disabled)
            q2 -= (1 << slice.event_left.node->layer); // special case
          values_[i][q].angle_rad = values_[i - 1][q2].angle_rad;
          values_[i][q].layer = values_[i - 1][q2].layer;
          values_[i][q].task = values_[i - 1][q2].task;
        }
      }
      if (values_[i][q].angle_rad < std::numeric_limits<double>::max()) continue;

      for (int layer = 0; layer < slice.tasks.size(); ++layer)
      {
        AnyOrderCycleNode::Ptr task = slice.tasks[layer];
        if (!task)
          continue;

        int k2 = (1 << layer);
        if ((k2 & q) == 0) continue;
        if (task->disabled) continue;

        double t1 = values_[i][q - k2].angle_rad;
        if (t1 == std::numeric_limits<double>::max()) continue;
        // special check
        if (!values_[i][q - k2].task->bead || values_[i][q - k2].task->bead->covering_radius_rad == 0.0)
        {
          if (layer != slices_[slice_index].event_left.node->layer) continue;
        }
        else t1 += task->bead->covering_radius_rad;

        t1 = std::max(t1, task->valid->from());
        if (t1 <= task->valid->to() && t1 + task->bead->covering_radius_rad < values_[i][q].angle_rad)
        {
          values_[i][q].angle_rad = t1 + task->bead->covering_radius_rad;
          values_[i][q].layer = layer;
          values_[i][q].task = task;
        }
      }

    }
  }


  const TaskSlice& slice = slices_[slice_index];
  if (values_[slices_.size() - 1][split2.Get()].angle_rad == std::numeric_limits<double>::max()) return false;
  if (values_[slices_.size() - 1][split2.Get()].angle_rad <= M_2xPI - slice.tasks[slice.event_left.node->layer]->bead->covering_radius_rad)
  {
    // feasible! construct solution
    int s = slices_.size() - 1;
    int s2 = (slice_index + s) % slices_.size();
    int q = split2.Get();  // TODO(tvl) remove q.
    double t = values_[s][q].angle_rad - values_[s][q].task->bead->covering_radius_rad;

    while (slices_[s2].coverage.from() > t + kEpsilon)
    {
      if (slices_[s2].event_left.type == TaskEvent::Type::kTo)
      {
        q += (1 << slices_[s2].event_left.node->layer);
        if (s > 0 && slices_[(s2 + slices_.size() - 1) % slices_.size()].tasks[slices_[s2].event_left.node->layer]->disabled)
          q -= (1 << slices_[s2].event_left.node->layer);
      }
      s--;
      s2 = (slice_index + s) % slices_.size();
      if (s < 0) break;
    }


    while (s >= 0 && values_[s][q].layer != -1)
    {
      //System.out.println(s + ", " + q);
      AnyOrderCycleNode::Ptr& task = values_[s][q].task;
      if ((q & (1 << values_[s][q].layer)) == 0) return false;
      q -= (1 << values_[s][q].layer);
      task->bead->angle_rad = t + slices_[slice_index].event_left.angle_rad;
      t = values_[s][q].angle_rad - (!values_[s][q].task->bead ? 0 : values_[s][q].task->bead->covering_radius_rad);
      while (slices_[s2].coverage.from() > t + kEpsilon)
      {
        if (slices_[s2].event_left.type == TaskEvent::Type::kTo)
        {
          q += (1 << slices_[s2].event_left.node->layer);
          if (s > 0 && slices_[(s2 + slices_.size() - 1) % slices_.size()].tasks[slices_[s2].event_left.node->layer]->disabled)
            q -= (1 << slices_[s2].event_left.node->layer);
        }
        s--;
        s2 = (slice_index + s) % slices_.size();
        if (s < 0) break;
      }
    }
    return true;
  }

  return false;
}


CheckFeasibleHeuristic::CheckFeasibleHeuristic(NodeSet& nodes, const int heuristic_cycles) :
  CheckFeasible(nodes),
  heuristic_cycles_(heuristic_cycles)
{}

bool CheckFeasibleHeuristic::operator()()
{
  if (slices_.empty())
    return true;

  ResetContainer();

  return FeasibleLine();
}

void CheckFeasibleHeuristic::InitializeSlices_()
{
  CheckFeasible::InitializeSlices_();

  // Clone the slices.
  const size_t num_slices = slices_.size();
  slices_.resize(num_slices * heuristic_cycles_);
  for (int cycle = 1; cycle < heuristic_cycles_; cycle++)
  {
    for (int j = 0; j < num_slices; j++)
    {
      int q = cycle * num_slices + j;
      slices_[q] = TaskSlice(slices_[j], cycle);
    }
  }
}

bool CheckFeasibleHeuristic::FeasibleLine()
{


  // initialization
  values_[0][0].angle_rad = 0.0;
  values_[0][0].angle2_rad = 0.0;
  values_[0][0].layer = -1;
  values_[0][0].task = std::make_shared<AnyOrderCycleNode>();

  for (int i = 0; i < slices_.size(); i++)
  {
    const TaskSlice& slice = slices_[i];
    for (int j = 0; j < slice.sets.size(); j++)
    {
      int q = slice.sets[j].Get();  // TODO(tvl) remove q.
      if (i == 0 && q == 0) continue;

      values_[i][q].angle_rad = std::numeric_limits<double>::max();
      values_[i][q].angle2_rad = values_[i][q].angle_rad;
      values_[i][q].layer = -1;
      values_[i][q].task = nullptr;

      if (i != 0)
      {
        // check previous slice
        if (slice.event_left.type == TaskEvent::Type::kFrom)
        {
          if ((q & (1 << slice.event_left.node->layer)) == 0)
          {
            values_[i][q].angle_rad = values_[i - 1][q].angle_rad;
            values_[i][q].angle2_rad = values_[i - 1][q].angle2_rad;
            values_[i][q].layer = values_[i - 1][q].layer;
            values_[i][q].task = values_[i - 1][q].task;
          }
        } else
        {
          int q2 = q + (1 << slice.event_left.node->layer);
          if (slices_[i - 1].tasks[slice.event_left.node->layer] == nullptr) q2 -= (1 << slice.event_left.node->layer); // special case
          values_[i][q].angle_rad = values_[i - 1][q2].angle_rad;
          values_[i][q].angle2_rad = values_[i - 1][q2].angle2_rad;
          values_[i][q].layer = values_[i - 1][q2].layer;
          values_[i][q].task = values_[i - 1][q2].task;
        }
      }

      if (values_[i][q].angle_rad < std::numeric_limits<double>::max()) continue;

      for (int layer = 0; layer < slice.tasks.size(); ++layer)
      {
        const AnyOrderCycleNode::Ptr& task = slice.tasks[layer];
        if (!task)
          continue;

        int k2 = (1 << layer);
        if ((k2 & q) == 0) continue;
        if (task == nullptr) continue;

        double t1 = values_[i][q - k2].angle_rad;
        if (t1 == std::numeric_limits<double>::max()) continue;

        double size = task->bead ? task->bead->covering_radius_rad : 0;
        // special check
        if (values_[i][q - k2].task->bead && values_[i][q - k2].task->bead->covering_radius_rad != 0.0)
          t1 += size;

        t1 = std::max(t1, task->valid->from());
        if (t1 <= task->valid->to() && t1 + size < values_[i][q].angle_rad)
        {
          values_[i][q].angle_rad = t1 + size;
          values_[i][q].angle2_rad = t1;
          values_[i][q].layer = layer;
          values_[i][q].task = task;
        }
      }

    }
  }


  NodeSet listCA;

  int s = slices_.size() - 1;
  int q = slices_[s].sets[slices_[s].sets.size() - 1].Get();  // TODO(tvl) remove q.
  double t = values_[s][q].angle_rad;
  if (t == std::numeric_limits<double>::max()) return false;
  t = values_[s][q].angle2_rad;

  while (slices_[s].coverage.from() > t + kEpsilon)
  {
    if (slices_[s].event_left.type == TaskEvent::Type::kTo)
    {
      q += (1 << slices_[s].event_left.node->layer);
      if (s > 0 && slices_[s - 1].tasks[slices_[s].event_left.node->layer] == nullptr) q -= (1 << slices_[s].event_left.node->layer);
    }
    s--;
    if (s < 0) break;
  }

  while (s >= 0 && values_[s][q].layer != -1)
  {
    AnyOrderCycleNode::Ptr& task = values_[s][q].task;
    q -= (1 << values_[s][q].layer);
    if (q < 0 || task == nullptr) return false;
    double size = task->bead ? task->bead->covering_radius_rad : 0;

    const Number angle_rad = t + slices_[0].event_left.angle_rad;
    listCA.push_back(std::make_shared<AnyOrderCycleNode>(task->bead));
    listCA.back()->valid = std::make_shared<Range>(angle_rad - size, angle_rad + size);
    listCA.back()->bead->angle_rad = angle_rad;

    t = values_[s][q].angle2_rad;
    while (slices_[s].coverage.from() > t + kEpsilon)
    {
      if (slices_[s].event_left.type == TaskEvent::Type::kTo)
      {
        q += (1 << slices_[s].event_left.node->layer);
        if (s > 0 && slices_[s - 1].tasks[slices_[s].event_left.node->layer] == nullptr) q -= (1 << slices_[s].event_left.node->layer);
      }
      s--;
      if (s < 0) break;
    }
  }


  // set to not found
  int count = 0;
  for (int i = 0; i < nodes_.size(); i++)
    nodes_[i]->bead->check = 0;

  int li = listCA.size() - 1;
  int ri = listCA.size() - 1;
  while (li >= 0 && listCA[li]->valid->to() <= listCA[ri]->valid->from() + M_2xPI)
  {
    Bead::Ptr c = listCA[li]->bead;
    c->check++;
    if (c->check == 1) count++;
    li--;
  }

  while (li >= 0)
  {
    if (count == nodes_.size()) break;
    AnyOrderCycleNode::Ptr& ca1 = listCA[li];
    AnyOrderCycleNode::Ptr& ca2 = listCA[ri];
    if (ca2->valid->from() + M_2xPI < ca1->valid->to())
    {
      ca2->bead->check--;
      if (ca2->bead->check == 0) count--;
      ri--;
    } else
    {
      ca1->bead->check++;
      if (ca1->bead->check == 1) count++;
      li--;
    }
  }
  li++;

  if (count == nodes_.size())
    return true;

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
  binary_search_depth_(binary_search_depth)
{
  // Collect and order the beads based on the start of their valid interval (initialized as their feasible interval).
  for (const Bead::Ptr& bead : necklace->beads)
    nodes_.push_back(std::make_shared<AnyOrderCycleNode>(bead));

  std::sort(nodes_.begin(), nodes_.end(), CompareAnyOrderCycleNode());

  // Prepare the feasibility check.
  check_feasible_ = CheckFeasible::New(nodes_, heuristic_cycles);
}

Number ComputeScaleFactorAnyOrder::Optimize()
{
  // Assign a layer to each node such that the nodes in a layer do not overlap in their feasibile intervals.
  const int num_layers = AssignLayers();

  // The algorithm is exponential in the number of layers, so we limit this number.
  if (kMaxLayers < num_layers)
    return 0;

  // Initialize the collection of task slices: collections of fixed tasks that are relevant within some angle range.
  check_feasible_->InitializeSlices();

  // Perform a binary search on the scale factor, determining which are feasible.
  // This binary search requires a decent initial upper bound on the scale factor.
  Number lower_bound = 0;
  Number upper_bound = ComputeScaleUpperBound();

  for (int step = 0; step < binary_search_depth_; ++step)
  {
    double scale_factor = 0.5 * (lower_bound + upper_bound);
    ComputeCoveringRadii(scale_factor);

    if ((*check_feasible_)())
      lower_bound = scale_factor;
    else
      upper_bound = scale_factor;
  }

  // The lower bound is the largest confirmed scale factor for which all beads could fit.
  return lower_bound;
}

Number ComputeScaleFactorAnyOrder::ComputeScaleUpperBound() const
{
  // The initial upper bound make sure all beads would fit if they were the size of the smallest bead.
  Number upper_bound = 0;
  for (const AnyOrderCycleNode::Ptr& node : nodes_)
  {
    const Number radius_rad = necklace_shape_->ComputeCoveringRadiusRad(node->valid, node->bead->radius_base);
    upper_bound = std::max(upper_bound, M_PI / (radius_rad + half_buffer_rad_));
  }

  // Perform a binary search to find the largest scale factor for which all beads could fit.
  Number lower_bound = 0.0;
  for (int step = 0; step < binary_search_depth_; ++step)
  {
    const Number scale_factor = 0.5 * (lower_bound + upper_bound);

    Number totalSize = 0.0;
    for (const AnyOrderCycleNode::Ptr& node : nodes_)
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
  for (AnyOrderCycleNode::Ptr& node : nodes_)
    node->bead->covering_radius_rad =
      necklace_shape_->ComputeCoveringRadiusRad(node->valid, node->bead->radius_base * scale_factor) + half_buffer_rad_;
}

int ComputeScaleFactorAnyOrder::AssignLayers()
{
  // Each node should be assigned a layer such that each layer does not contain any pair of nodes that overlap in their valid interval.
  // Note that this can be done greedily: assign the nodes by minimizing the distance between the last valid interval and the next.

  using NodeList = std::list<AnyOrderCycleNode::Ptr>;
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
    }
    else if (node_iter == unused_iter)
    {
      // All nodes were checked: start a new layer.
      ++layer;
      (*node_iter)->layer = layer;
      layer_interval = NecklaceInterval(*(*node_iter)->valid);

      node_iter = remaining_nodes.erase(node_iter);
      unused_iter = remaining_nodes.end();
    }
    else
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









ComputeScaleFactorAnyOrderIngot::ComputeScaleFactorAnyOrderIngot
(
  const Necklace::Ptr& necklace,
  const Number& buffer_rad /*= 0*/,
  const int binary_search_depth /*= 10*/,
  const int heuristic_cycles /*= 5*/
) : ComputeScaleFactorAnyOrder(necklace, buffer_rad, binary_search_depth, heuristic_cycles)
{}

Number ComputeScaleFactorAnyOrderIngot::ComputeScaleUpperBound() const
{
  return M_PI / nodes_.size() - half_buffer_rad_;
}

Number ComputeScaleFactorAnyOrderIngot::ComputeCoveringRadii(const Number& scale_factor)
{
  for (AnyOrderCycleNode::Ptr& node : nodes_)
    node->bead->covering_radius_rad = scale_factor + half_buffer_rad_;
}

} // namespace detail
} // namespace necklace_map
} // namespace geoviz
