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

constexpr const Number EPSILON = 0.0000001;


AnyOrderCycleNode::AnyOrderCycleNode(const Bead::Ptr& bead) :
  CycleNode(bead), layer(-1)
{}

AnyOrderCycleNode::AnyOrderCycleNode
(
  const Bead::Ptr& bead,
  const Number& angle_rad,
  const Number& buffer_rad
) :
  CycleNode(bead, std::make_shared<Range>(angle_rad - buffer_rad, angle_rad + buffer_rad)),
  layer(-1)
{
  bead->angle_rad = angle_rad;
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







BeadData::BeadData(const Bead::Ptr& bead, const int layer) :
  bead(bead),
  layer(layer),
  disabled(bead)
{
  if (bead)
    valid = std::make_shared<Range>(*bead->feasible);
  else
    valid = std::make_shared<Range>(0, 0);
}

BeadData::BeadData(const BeadData& data)
{
  bead = data.bead;
  layer = data.layer;
  disabled = false;
  valid = std::make_shared<Range>(*data.valid);
}






bool BitString::CheckFit(const int bit) { return bit < 32; }

void BitString::SetBit(const int bit) { CHECK(CheckFit(bit)); bits = 1 << bit; }

bool BitString::AddBit(const int bit) { CHECK(CheckFit(bit)); return bits |= (1 << bit); }


TaskSlice::TaskSlice() :
  event_left(),
  event_right(),
  tasks(),
  num_tasks(0),
  angle_left_rad(0),
  angle_right_rad(0),
  sets(),
  layers()
{}

TaskSlice::TaskSlice(
  const TaskEvent& event_left,
  const TaskEvent& event_right,
  const int num_layers,
  const double angle_right_rad
) :
  event_left(event_left),
  event_right(event_right),
  num_tasks(0),
  angle_left_rad(event_left.angle_rad),
  angle_right_rad(angle_right_rad)
{
  tasks.resize(num_layers);
}

TaskSlice::TaskSlice(const TaskSlice& ts, const double offset, const int step) :
  event_left(ts.event_left),
  event_right(ts.event_right),
  num_tasks(ts.num_tasks),
  sets(ts.sets),
  layers(ts.layers)
{
  angle_left_rad = event_left.angle_rad - offset + step * M_2xPI;
  if (angle_left_rad < step * M_2xPI) angle_left_rad += M_2xPI;
  angle_right_rad = event_right.angle_rad - offset + step * M_2xPI;
  if (angle_right_rad < angle_left_rad) angle_right_rad += M_2xPI;

  tasks.resize(ts.tasks.size());
  for (int i = 0; i < tasks.size(); i++)
  {
    if (ts.tasks[i] == nullptr) tasks[i] = nullptr;
    else
    {
      //if (ts.tasks[i].range.from <= ts.tasks[i].range.to || step > 0 || angle_right_rad > ts.tasks[i].range.from) {
      if (step > 0 || angle_right_rad > ts.tasks[i]->valid->to() - offset || !ts.tasks[i]->valid->Contains(offset) ||
          ts.tasks[i]->valid->from() == offset)
      {
        tasks[i] = std::make_shared<BeadData>(*ts.tasks[i]);  // TODO(tvl) check whether this should be a new pointer or could also be a pointer to the existing object.
        Range r1(tasks[i]->valid->from(), event_left.angle_rad);
        Range r2(event_left.angle_rad, tasks[i]->valid->to());
        tasks[i]->valid->from() = angle_left_rad - r1.ComputeLength();
        tasks[i]->valid->to() = angle_left_rad + r2.ComputeLength();
      } else tasks[i] = nullptr;
    }
  }
}

void TaskSlice::reset()
{
  angle_left_rad = event_left.angle_rad;
  angle_right_rad = event_right.angle_rad;
  for (int i = 0; i < tasks.size(); i++)
  {
    if (tasks[i] != nullptr)
    {
      BeadData::Ptr cd = tasks[i];
      cd->valid = std::make_shared<Range>(*cd->bead->feasible);
      cd->disabled = false;
    }
  }
}

void TaskSlice::rotate(const Number value, const std::vector<BeadData::Ptr>& cds, const BitString& split)
{
  Range r1(value, angle_left_rad);
  Range r2(value, angle_right_rad);
  angle_left_rad = r1.ComputeLength();
  angle_right_rad = r2.ComputeLength();
  if (angle_right_rad < EPSILON) angle_right_rad = M_2xPI;

  // TODO(tvl) when changing r1/r2, only set the second value?
  for (int i = 0; i < tasks.size(); i++)
  {
    if (tasks[i] != nullptr)
    {
      BeadData::Ptr cd = tasks[i];
      if (cds[i] != nullptr && cds[i]->bead == cd->bead)
      {
        if (split.HasBit(i))
        {
          r2 = Range(value, cd->valid->to());
          cd->valid->from() = 0.0;
          cd->valid->to() = r2.ComputeLength();
          if (r2.ComputeLength() - EPSILON <= angle_left_rad)
            cd->disabled = true;
        } else
        {
          r1 = Range(value, cd->valid->from());
          cd->valid->from() = r1.ComputeLength();
          cd->valid->to() = M_2xPI;
          if (r1.ComputeLength() + EPSILON >= angle_right_rad)
            cd->disabled = true;
        }
      } else
      {
        r1 = Range(value, cd->valid->from());
        r2 = Range(value, cd->valid->to());
        cd->valid->from() = r1.ComputeLength();
        cd->valid->to() = r2.ComputeLength();
        if (cd->valid->to() < EPSILON)
          cd->valid->to() = M_2xPI;
      }
    }
  }
}

void TaskSlice::addTask(const BeadData::Ptr& task)
{
  CHECK_LT(task->layer, tasks.size());
  tasks[task->layer] = task;
  num_tasks++;
  CHECK(BitString::CheckFit(num_tasks));
}

void TaskSlice::produceSets()
{
  sets.resize(1 << num_tasks);  // TODO(tvl) bitset?
  std::vector<bool> filter(tasks.size());
  for (int i = 0; i < tasks.size(); i++)
    filter[i] = (tasks[i] != nullptr);
  int n = (1 << tasks.size());
  int k = 0;
  for (int i = 0; i < n; i++)
  {
    bool valid = true;
    for (int j = 0, q = 1; j < tasks.size(); j++, q = (q << 1))
    {
      if ((q & i) > 0 && !filter[j]) valid = false;
    }
    if (valid) sets[k++].SetString(i);
  }

  layers.resize(num_tasks);
  k = 0;
  for (int i = 0; i < tasks.size(); i++)
  {
    if (tasks[i] != nullptr)
      layers[k++] = i;
  }
}


OptValue::OptValue()
{
  Initialize();
}

void OptValue::Initialize()
{
  angle_rad = std::numeric_limits<double>::max();
  angle2_rad = std::numeric_limits<double>::max();
  layer = -1;
  cd = nullptr;
}


ComputeScaleFactorAnyOrder::ComputeScaleFactorAnyOrder
(
  const Necklace::Ptr& necklace,
  const Number& buffer_rad /*= 0*/,
  const int binary_search_depth /*= 10*/,
  const int heuristic_steps /*= 5*/
) :
  necklace_shape_(necklace->shape),
  half_buffer_rad_(0.5 * buffer_rad),
  binary_search_depth_(binary_search_depth),
  heuristic_steps_(heuristic_steps)
{
  // Collect and order the beads based on the start of their valid interval (initialized as their feasible interval).
  for (const Bead::Ptr& bead : necklace->beads)
    nodes_.push_back(std::make_shared<AnyOrderCycleNode>(bead));

  std::sort(nodes_.begin(), nodes_.end(), CompareAnyOrderCycleNode());
}

Number ComputeScaleFactorAnyOrder::Optimize()
{
  const int num_layers = AssignLayers();


  // TODO(tvl) check which vectors can be replaced by simple fixed size arrays.




  // Failure case: too thick.
  if (num_layers >= 15) return 0;

  // compute upper bound scale
  const Number maxScale = ComputeScaleUpperBound();

  // TODO(tvl) why does this repeat the earlier initialization? layer is set...
  // TODO(tvl) this recreation of the event list could be replaced by a link in the events to the CA...
  std::vector<TaskEvent> events;  // TODO(tvl) reserve
  for (int i = 0; i < nodes_.size(); i++)
  {
    AnyOrderCycleNode::Ptr& ca = nodes_[i];
    events.emplace_back(ca, ca->valid->from(), TaskEvent::Type::kFrom);
    events.emplace_back(ca, ca->valid->to(), TaskEvent::Type::kTo);
  }
  std::sort(events.begin(), events.end(), CompareTaskEvent());  // TODO(tvl) is this even useful?


  Bead::Ptr curTasks[num_layers];
  // initialize
  for (const AnyOrderCycleNode::Ptr& node : nodes_)
  {
    if (node->valid->Contains(0) && node->valid->to() < M_2xPI)
      curTasks[node->layer] = node->bead;
  }

  //find taskslices
  std::vector<TaskSlice> slices(events.size());
  for (int i = 0; i < events.size(); i++)
  {
    TaskEvent e = events[i];
    TaskEvent e2 = events[(i + 1) % events.size()];
    slices[i] = TaskSlice(e, e2, num_layers, e2.angle_rad);
    if (e.type == TaskEvent::Type::kFrom) curTasks[e.node->layer] = e.node->bead;
    else curTasks[e.node->layer] = nullptr;
    for (int j = 0; j < num_layers; j++)
      if (curTasks[j] != nullptr)
        slices[i].addTask(std::make_shared<BeadData>(curTasks[j], j));
  }

  for (TaskSlice& slice : slices)
    slice.produceSets();

  // make sure first slice is start of task
  while (slices[0].event_left.type == TaskEvent::Type::kTo)
  {
    const TaskSlice& ts = slices[0];
    for (int i = 0; i < slices.size() - 1; i++)
      slices[i] = slices[i + 1];
    slices[slices.size() - 1] = ts;
  }



  // binary search
  Number x = 0.0;
  Number y = maxScale;
  for (int i = 0; i < binary_search_depth_; i++)
  {
    double h = 0.5 * (x + y);
    ComputeCoveringRadii(h);



    if (heuristic_steps_ == 0)  // TODO(tvl) Merge in method that switches between feasible methods based on heuristic steps.
    {
      if (feasible(slices, num_layers)) x = h;
      else y = h;
    } else
    {
      if (feasible2(slices, num_layers, heuristic_steps_)) x = h;
      else y = h;
    }
  }
  //System.out.println(x);
  return x;
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
  for (int j = 0; j < binary_search_depth_; ++j)
  {
    Number scale_factor = 0.5 * (lower_bound + upper_bound);

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





bool ComputeScaleFactorAnyOrder::feasible
(
  std::vector<TaskSlice>& slices,
  const int num_layers
)
{

  // setup DP array
  int nSubSets = (1 << num_layers);
  std::vector<std::vector<OptValue> > opt(slices.size());
  for (int i = 0; i < slices.size(); i++)
  {
    opt[i].resize(nSubSets);
    // TODO(tvl) check whether default initialization is done correctly...
    /*for (int j = 0; j < nSubSets; j++) {
      opt[i][j].Initialize();
    }*/
  }

  // try all possibilities
  for (int i = 0; i < slices.size(); i++)
  {
    if (slices[i].event_left.type == TaskEvent::Type::kFrom)
    {
      int q = (1 << slices[i].event_left.node->layer);
      for (int j = 0; j < slices[i].sets.size(); j++)
      {
        const BitString& str2 = slices[i].sets[j];
        int q2 = str2.Get();
        if (str2.HasBit(q))
        {

          // split the circle (ranges, event times, the works)
          splitCircle(slices, i, str2);

          // compute
          bool good = feasibleLine(slices, opt, i, str2);

          if (good) return true;
        }
      }
    }
  }

  return false;
}

void ComputeScaleFactorAnyOrder::splitCircle
(
  std::vector<TaskSlice>& slices,
  const int slice,
  const BitString& split
)
{
  // reset everything, then rotate
  for (int i = 0; i < slices.size(); i++)
  {
    slices[i].reset();
    slices[i].rotate(slices[slice].event_left.angle_rad, slices[slice].tasks, split);
  }
}

bool ComputeScaleFactorAnyOrder::feasibleLine
(
  const std::vector<TaskSlice>& slices,
  std::vector<std::vector<OptValue> >& opt,
  const int slice,
  const BitString& split
)
{
  BitString split2 = split.Xor(slices[slice].sets.back());  //TODO(tvl) rename split_inverse?

  // initialization
  //const TaskSlice& ts = slices[slice];
  opt[0][0].angle_rad = 0.0;
  opt[0][0].layer = -1;
  opt[0][0].cd = std::make_shared<BeadData>(nullptr, -1);

  for (int i = 0; i < slices.size(); i++)
  {
    int s = (slice + i) % slices.size();
    const TaskSlice& ts = slices[s];
    for (int j = 0; j < ts.sets.size(); j++)
    {
      const BitString& str = ts.sets[j];
      int q = str.Get();  // TODO(tvl) remove q.
      if (i == 0 && q == 0) continue;

      opt[i][q].angle_rad = std::numeric_limits<double>::max();
      opt[i][q].layer = -1;
      opt[i][q].cd = nullptr;

      if (i == 0 && split2.HasAny(str)) continue;
      if (i == slices.size() - 1 && split.HasAny(str)) continue;

      if (i != 0)
      {
        // check previous slice
        if (ts.event_left.type == TaskEvent::Type::kFrom)
        {
          if ((q & (1 << ts.event_left.node->layer)) == 0)
          {
            opt[i][q].angle_rad = opt[i - 1][q].angle_rad;
            opt[i][q].layer = opt[i - 1][q].layer;
            opt[i][q].cd = opt[i - 1][q].cd;
          }
        } else
        {
          int q2 = q + (1 << ts.event_left.node->layer);
          if (slices[(s + slices.size() - 1) % slices.size()].tasks[ts.event_left.node->layer]->disabled)
            q2 -= (1 << ts.event_left.node->layer); // special case
          opt[i][q].angle_rad = opt[i - 1][q2].angle_rad;
          opt[i][q].layer = opt[i - 1][q2].layer;
          opt[i][q].cd = opt[i - 1][q2].cd;
        }
      }
      if (opt[i][q].angle_rad < std::numeric_limits<double>::max()) continue;

      for (int x = 0; x < ts.num_tasks; x++)
      {
        int k = ts.layers[x];
        BeadData::Ptr cd = ts.tasks[k];
        int k2 = (1 << k);
        if ((k2 & q) == 0) continue;
        if (cd->disabled) continue;

        double t1 = opt[i][q - k2].angle_rad;
        if (t1 == std::numeric_limits<double>::max()) continue;
        // special check
        if (!opt[i][q - k2].cd->bead || opt[i][q - k2].cd->bead->covering_radius_rad == 0.0)
        {
          if (k != slices[slice].event_left.node->layer) continue;
        }
        else t1 += cd->bead->covering_radius_rad;

        t1 = std::max(t1, cd->valid->from());
        if (t1 <= cd->valid->to() && t1 + cd->bead->covering_radius_rad < opt[i][q].angle_rad)
        {
          opt[i][q].angle_rad = t1 + cd->bead->covering_radius_rad;
          opt[i][q].layer = k;
          opt[i][q].cd = cd;
        }
      }

    }
  }


  const TaskSlice& ts = slices[slice];
  if (opt[slices.size() - 1][split2.Get()].angle_rad == std::numeric_limits<double>::max()) return false;
  if (opt[slices.size() - 1][split2.Get()].angle_rad <= M_2xPI - ts.tasks[ts.event_left.node->layer]->bead->covering_radius_rad)
  {
    // feasible! construct solution
    int s = slices.size() - 1;
    int s2 = (slice + s) % slices.size();
    int q = split2.Get();  // TODO(tvl) remove q.
    double t = opt[s][q].angle_rad - opt[s][q].cd->bead->covering_radius_rad;

    while (slices[s2].angle_left_rad > t + EPSILON)
    {
      if (slices[s2].event_left.type == TaskEvent::Type::kTo)
      {
        q += (1 << slices[s2].event_left.node->layer);
        if (s > 0 && slices[(s2 + slices.size() - 1) % slices.size()].tasks[slices[s2].event_left.node->layer]->disabled)
          q -= (1 << slices[s2].event_left.node->layer);
      }
      s--;
      s2 = (slice + s) % slices.size();
      if (s < 0) break;
    }


    while (s >= 0 && opt[s][q].layer != -1)
    {
      //System.out.println(s + ", " + q);
      BeadData::Ptr cd = opt[s][q].cd;
      if ((q & (1 << opt[s][q].layer)) == 0) return false;
      q -= (1 << opt[s][q].layer);
      cd->bead->angle_rad = t + slices[slice].event_left.angle_rad;
      t = opt[s][q].angle_rad - opt[s][q].cd->bead->covering_radius_rad;
      while (slices[s2].angle_left_rad > t + EPSILON)
      {
        if (slices[s2].event_left.type == TaskEvent::Type::kTo)
        {
          q += (1 << slices[s2].event_left.node->layer);
          if (s > 0 && slices[(s2 + slices.size() - 1) % slices.size()].tasks[slices[s2].event_left.node->layer]->disabled)
            q -= (1 << slices[s2].event_left.node->layer);
        }
        s--;
        s2 = (slice + s) % slices.size();
        if (s < 0) break;
      }
    }
    return true;
  }

  return false;
}

bool ComputeScaleFactorAnyOrder::feasible2  // TODO(tvl) rename feasible heuristic.
(
  const std::vector<TaskSlice>& slices,
  const int num_layers,
  const int copies
)
{
  // make new slices
  std::vector<TaskSlice> slices2(slices.size() * copies);
  for (int i = 0; i < copies; i++)
  {
    for (int j = 0; j < slices.size(); j++)
    {
      int q = i * slices.size() + j;
      slices2[q] = TaskSlice(slices[j], slices[0].angle_left_rad, i);
    }
  }

  // setup DP array
  int nSubSets = (1 << num_layers);
  std::vector<std::vector<OptValue> > opt(slices2.size());
  for (int i = 0; i < slices2.size(); i++)
    opt[i].resize(nSubSets);

  return feasibleLine2(slices2, opt);
}

bool ComputeScaleFactorAnyOrder::feasibleLine2
(
  const std::vector<TaskSlice>& slices,
  std::vector<std::vector<OptValue> >& opt
)
{
  // initialization
  //const TaskSlice& ts = slices[0];
  opt[0][0].angle_rad = 0.0;
  opt[0][0].angle2_rad = 0.0;
  opt[0][0].layer = -1;
  opt[0][0].cd = std::make_shared<BeadData>(nullptr, -1);

  for (int i = 0; i < slices.size(); i++)
  {
    const TaskSlice& ts = slices[i];
    for (int j = 0; j < ts.sets.size(); j++)
    {
      int q = ts.sets[j].Get();  // TODO(tvl) remove q.
      if (i == 0 && q == 0) continue;

      opt[i][q].angle_rad = std::numeric_limits<double>::max();
      opt[i][q].angle2_rad = opt[i][q].angle_rad;
      opt[i][q].layer = -1;
      opt[i][q].cd = nullptr;

      if (i != 0)
      {
        // check previous slice
        if (ts.event_left.type == TaskEvent::Type::kFrom)
        {
          if ((q & (1 << ts.event_left.node->layer)) == 0)
          {
            opt[i][q].angle_rad = opt[i - 1][q].angle_rad;
            opt[i][q].angle2_rad = opt[i - 1][q].angle2_rad;
            opt[i][q].layer = opt[i - 1][q].layer;
            opt[i][q].cd = opt[i - 1][q].cd;
          }
        } else
        {
          int q2 = q + (1 << ts.event_left.node->layer);
          if (slices[i - 1].tasks[ts.event_left.node->layer] == nullptr) q2 -= (1 << ts.event_left.node->layer); // special case
          opt[i][q].angle_rad = opt[i - 1][q2].angle_rad;
          opt[i][q].angle2_rad = opt[i - 1][q2].angle2_rad;
          opt[i][q].layer = opt[i - 1][q2].layer;
          opt[i][q].cd = opt[i - 1][q2].cd;
        }
      }

      if (opt[i][q].angle_rad < std::numeric_limits<double>::max()) continue;

      for (int x = 0; x < ts.num_tasks; x++)
      {
        int k = ts.layers[x];
        BeadData::Ptr cd = ts.tasks[k];
        int k2 = (1 << k);
        if ((k2 & q) == 0) continue;
        if (cd == nullptr) continue;

        double t1 = opt[i][q - k2].angle_rad;
        if (t1 == std::numeric_limits<double>::max()) continue;

        double size = cd->bead ? cd->bead->covering_radius_rad : 0;
        // special check
        if (opt[i][q - k2].cd->bead && opt[i][q - k2].cd->bead->covering_radius_rad != 0.0)
          t1 += size;

        t1 = std::max(t1, cd->valid->from());
        if (t1 <= cd->valid->to() && t1 + size < opt[i][q].angle_rad)
        {
          opt[i][q].angle_rad = t1 + size;
          opt[i][q].angle2_rad = t1;
          opt[i][q].layer = k;
          opt[i][q].cd = cd;
        }
      }

    }
  }


  NodeSet listCA;

  int s = slices.size() - 1;
  int q = slices[s].sets[slices[s].sets.size() - 1].Get();  // TODO(tvl) remove q.
  double t = opt[s][q].angle_rad;
  if (t == std::numeric_limits<double>::max()) return false;
  //t -= opt[s][q].cd.size;
  t = opt[s][q].angle2_rad;

  while (slices[s].angle_left_rad > t + EPSILON)
  {
    if (slices[s].event_left.type == TaskEvent::Type::kTo)
    {
      q += (1 << slices[s].event_left.node->layer);
      if (s > 0 && slices[s - 1].tasks[slices[s].event_left.node->layer] == nullptr) q -= (1 << slices[s].event_left.node->layer);
    }
    s--;
    if (s < 0) break;
  }

  while (s >= 0 && opt[s][q].layer != -1)
  {
    BeadData::Ptr cd = opt[s][q].cd;
    q -= (1 << opt[s][q].layer);
    if (q < 0 || cd == nullptr) return false;
    double size = cd->bead ? cd->bead->covering_radius_rad : 0;
    listCA.push_back(std::make_shared<AnyOrderCycleNode>(cd->bead, t + slices[0].event_left.angle_rad, size));
    t = opt[s][q].angle2_rad;
    while (slices[s].angle_left_rad > t + EPSILON)
    {
      if (slices[s].event_left.type == TaskEvent::Type::kTo)
      {
        q += (1 << slices[s].event_left.node->layer);
        if (s > 0 && slices[s - 1].tasks[slices[s].event_left.node->layer] == nullptr) q -= (1 << slices[s].event_left.node->layer);
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


ComputeScaleFactorAnyOrderIngot::ComputeScaleFactorAnyOrderIngot
(
  const Necklace::Ptr& necklace,
  const Number& buffer_rad /*= 0*/,
  const int binary_search_depth /*= 10*/,
  const int heuristic_steps /*= 5*/
) : ComputeScaleFactorAnyOrder(necklace, buffer_rad, binary_search_depth, heuristic_steps)
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
