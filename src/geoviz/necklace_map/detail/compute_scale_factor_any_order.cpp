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


CountryData::CountryData(const Bead::Ptr& bead, const int layer) :
  bead(bead),
  radius_cur(bead ? bead->radius_base : 0),
  layer(layer),
  disabled(bead)
{
  if (bead)
    range_cur = std::make_shared<Range>(*bead->feasible);
  else
    range_cur = std::make_shared<Range>(0, 0);
}

CountryData::CountryData(const CountryData& cd)
{
  bead = cd.bead;
  layer = cd.layer;
  disabled = false;
  radius_cur = cd.radius_cur;
  range_cur = std::make_shared<Range>(*cd.range_cur);
}

double CountryData::lookUpSize(double angle)
{
  /*Range r(range_cur->from_rad(), angle);
  int k = (int)std::round(r.ComputeLength() / LOOKUP_TABLE_STEP);
  if (k >= 0 && k < c.lookUpSize.length) return c.lookUpSize[k];
  else*/
  return radius_cur;
}


TaskSlice::TaskSlice() : eventLeft(), eventRight(), tasks(), taskCount(0), left(0), right(0), sets(), layers() {}

TaskSlice::TaskSlice(
  const TaskEvent& eLeft,
  const TaskEvent& eRight,
  const int K,
  const double right
) :  // TODO(tvl) note Java code had bug here, where 'right' was not actually used; is this parameter needed?
  eventLeft(eLeft),
  eventRight(eRight),
  taskCount(0),
  left(eLeft.angle_rad),
  right(right)
{
  tasks.resize(K);
}

TaskSlice::TaskSlice(const TaskSlice& ts, const double offset, const int step) :
  eventLeft(ts.eventLeft),
  eventRight(ts.eventRight),
  taskCount(ts.taskCount),
  sets(ts.sets),
  layers(ts.layers)
{
  left = eventLeft.angle_rad - offset + step * M_2xPI;
  if (left < step * M_2xPI) left += M_2xPI;
  right = eventRight.angle_rad - offset + step * M_2xPI;
  if (right < left) right += M_2xPI;

  tasks.resize(ts.tasks.size());
  for (int i = 0; i < tasks.size(); i++)
  {
    if (ts.tasks[i] == nullptr) tasks[i] = nullptr;
    else
    {
      //if (ts.tasks[i].range.from <= ts.tasks[i].range.to || step > 0 || right > ts.tasks[i].range.from) {
      if (step > 0 || right > ts.tasks[i]->range_cur->to() - offset || !ts.tasks[i]->range_cur->Contains(offset) ||
          ts.tasks[i]->range_cur->from() == offset)
      {
        tasks[i] = std::make_shared<CountryData>(*ts.tasks[i]);  // TODO(tvl) check whether this should be a new pointer or could also be a pointer to the existing object.
        Range r1(tasks[i]->range_cur->from(), eventLeft.angle_rad);
        Range r2(eventLeft.angle_rad, tasks[i]->range_cur->to());
        tasks[i]->range_cur->from() = left - r1.ComputeLength();
        tasks[i]->range_cur->to() = left + r2.ComputeLength();
      } else tasks[i] = nullptr;
    }
  }
}

void TaskSlice::reset()
{
  left = eventLeft.angle_rad;
  right = eventRight.angle_rad;
  for (int i = 0; i < tasks.size(); i++)
  {
    if (tasks[i] != nullptr)
    {
      CountryData::Ptr cd = tasks[i];
      cd->range_cur = std::make_shared<Range>(*cd->bead->feasible);
      cd->disabled = false;
    }
  }
}

void TaskSlice::rotate(const double value, const std::vector<CountryData::Ptr>& cds, const int split)
{
  Range r1(value, left);
  Range r2(value, right);
  left = r1.ComputeLength();
  right = r2.ComputeLength();
  if (right < EPSILON) right = M_2xPI;

  // TODO(tvl) when changing r1/r2, only set the second value?
  for (int i = 0; i < tasks.size(); i++)
  {
    if (tasks[i] != nullptr)
    {
      CountryData::Ptr cd = tasks[i];
      if (cds[i] != nullptr && cds[i]->bead == cd->bead)
      {
        if (((1 << i) & split) > 0)
        {
          r2 = Range(value, cd->range_cur->to());
          cd->range_cur->from() = 0.0;
          cd->range_cur->to() = r2.ComputeLength();
          if (r2.ComputeLength() - EPSILON <= left)
            cd->disabled = true;
        } else
        {
          r1 = Range(value, cd->range_cur->from());
          cd->range_cur->from() = r1.ComputeLength();
          cd->range_cur->to() = M_2xPI;
          if (r1.ComputeLength() + EPSILON >= right)
            cd->disabled = true;
        }
      } else
      {
        r1 = Range(value, cd->range_cur->from());
        r2 = Range(value, cd->range_cur->to());
        cd->range_cur->from() = r1.ComputeLength();
        cd->range_cur->to() = r2.ComputeLength();
        if (cd->range_cur->to() < EPSILON)
          cd->range_cur->to() = M_2xPI;
      }
    }
  }
}

void TaskSlice::addTask(const CountryData::Ptr& task)
{
  CHECK_LT(task->layer, tasks.size());
  tasks[task->layer] = task;
  taskCount++;
}

void TaskSlice::produceSets()
{
  sets.resize(1 << taskCount);  // TODO(tvl) bitset?
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
    if (valid) sets[k++] = i;
  }

  layers.resize(taskCount);
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


double Optimizer::computeOptSize
(
  const double bufferSize/*rename buffer_rad*/,
  const int precision,
  const Necklace::Ptr& cg/*rename: necklace*/,
  const int heurSteps, // heurSteps == 0 -> Exact algo
  const bool ingot  // TODO(tvl) remove.
)
{
  // TODO(tvl) check which vectors can be replaced by simple fixed size arrays.


  // gather Country range pairs
  // Note that the country sympbolAngle and range have been set by the interval algorithm.
  cas.clear();  // TODO(tvl) reserve
  for (const Bead::Ptr& bead : cg->beads)
  {
    if (bead->radius_base <= 0)
      continue;

    cas.push_back(std::make_shared<AnyOrderCycleNode>(bead));
  }
  std::sort(cas.begin(), cas.end(), CompareAnyOrderCycleNode());

  // assign tasks to layers
  std::vector<TaskEvent> events;  // TODO(tvl) reserve
  for (size_t i = 0; i < cas.size(); i++)
  {
    AnyOrderCycleNode::Ptr& ca = cas[i];
    events.emplace_back(ca, ca->valid->from(), TaskEvent::Type::kFrom);
    events.emplace_back(ca, ca->valid->to(), TaskEvent::Type::kTo);
  }
  std::sort(events.begin(), events.end(), CompareTaskEvent());

  // find angle with max thickness
  int K = 0;
  double angle = cas[0]->valid->from();
  for (const AnyOrderCycleNode::Ptr& node : cas)
    if (node->valid->Contains(angle))
      K++;
  int optK = K;
  AnyOrderCycleNode::Ptr opt_node = 0;
  for (const TaskEvent& event : events)
  {
    if (event.angle_rad <= angle) continue;
    if (event.type == TaskEvent::Type::kFrom)
    {
      K++;
      if (K > optK)
      {
        optK = K;
        angle = event.angle_rad;
        opt_node = event.node;
      }
    } else K--;
  }

  // Find a non-overlapping order of ca's (where the next ca is the closest one to the current one.)
  CountryAngleSet L;
  L.reserve(cas.size());
  int count = cas.size() - 1;
  Range::Ptr curRange = opt_node->valid;
  opt_node->layer = 0;
  L.push_back(opt_node);
  while (count > 0)
  {
    AnyOrderCycleNode::Ptr next_node;
    double minAngleDif = M_2xPI;

    for (int i = 0; i < cas.size(); i++)
    {
      if (cas[i]->layer >= 0) continue;
      Range r(curRange->to(), cas[i]->valid->from());
      const Number length_rad = r.ComputeLength();
      if (length_rad < minAngleDif)
      {
        minAngleDif = length_rad;
        next_node = cas[i];
      }
    }
    count--;
    L.push_back(next_node);
    next_node->layer = 0;
    curRange = next_node->valid;
  }

  // now really assign layers
  K = 1;
  for (int i = 0; i < L.size(); i++)
  {
    bool good = true;
    AnyOrderCycleNode::Ptr& k1 = L[i];
    for (int j = 0; j < i; j++)
    { // TODO(tvl) this may be done more efficiently (as opposed to a full double loop)?
      AnyOrderCycleNode::Ptr& k2 = L[j];
      if (k2->layer != K - 1) continue;
      if (k2->valid->Intersects(k1->valid)) good = false;
    }
    if (!good)
    {
      k1->layer = K;
      K++;
    } else k1->layer = K - 1;
  }

  // Failure case: too thick.
  if (K >= 15) return 0;
  //System.out.println(cas.size());
  //System.out.println(K);

  // TODO(tvl) why does this repeat the earlier initialization? layer is set...
  // TODO(tvl) this recreation of the event list could be replaced by a link in the events to the CA...
  /*events.clear();
  for (int i = 0; i < cas.size(); i++)
  {
    AnyOrderCycleNode::Ptr& ca = cas[i];
    events.emplace_back(ca, ca->valid->from(), true);
    events.emplace_back(ca, ca->valid->to(), false);
  }*/
  std::sort(events.begin(), events.end(), CompareTaskEvent());  // TODO(tvl) is this even useful?


  Bead::Ptr curTasks[K];
  // initialize
  for (const AnyOrderCycleNode::Ptr& node : cas)
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
    slices[i] = TaskSlice(e, e2, K, e2.angle_rad);
    if (e.type == TaskEvent::Type::kFrom) curTasks[e.node->layer] = e.node->bead;
    else curTasks[e.node->layer] = nullptr;
    for (int j = 0; j < K; j++)
      if (curTasks[j] != nullptr)
        slices[i].addTask(std::make_shared<CountryData>(curTasks[j], j));
  }

  for (TaskSlice& slice : slices)
    slice.produceSets();

  // make sure first slice is start of task
  while (slices[0].eventLeft.type == TaskEvent::Type::kTo)
  {
    const TaskSlice& ts = slices[0];
    for (int i = 0; i < slices.size() - 1; i++)
      slices[i] = slices[i + 1];
    slices[slices.size() - 1] = ts;
  }

  // compute upper bound scale
  double maxScale = -1;
  double length = cg->shape->ComputeLength();
  for (int i = 0; i < cas.size(); i++)
  {
    double max_bead_scale = length / (2.0 * cas[i]->bead->radius_base);
    if (maxScale < 0 || max_bead_scale < maxScale)
      maxScale = max_bead_scale;
  }
  if (ingot)
    maxScale = M_PI / cas.size();  // TODO(tvl) even with ingot mode enabled: why do the previous calculations in this case?

  // 1st binary search
  double x = 0.0;
  double y = maxScale;
  for (int j = 0; j < 10; j++)
  {
    double h = 0.5 * (x + y);
    double totalSize = 0.0;
    for (int i = 0; i < cas.size(); i++)
    {
      if (!ingot)
        totalSize += cg->shape->ComputeCoveringSize(cas[i]->valid, cas[i]->bead->radius_base * h) + bufferSize;
      else totalSize += h + bufferSize;
    }
    if (totalSize <= M_PI) x = h;
    else y = h;
  }
  maxScale = x;

  // binary search
  x = 0.0;
  y = maxScale;
  for (int i = 0; i < precision; i++)
  {
    double h = 0.5 * (x + y);
    if (heurSteps == 0)
    {
      if (feasible(slices, h, K, bufferSize, cg, ingot)) x = h;
      else y = h;
    } else
    {
      if (feasible2(slices, h, K, bufferSize, cg, heurSteps, ingot)) x = h;
      else y = h;
    }
  }
  //System.out.println(x);
  return x;
}

bool Optimizer::feasible
(
  std::vector<TaskSlice>& slices,
  const double scale,
  const int K,
  const double bufferSize,
  const Necklace::Ptr& necklace,
  const bool ingot
)
{
  // set sizes
  for (int i = 0; i < cas.size(); i++)
  {
    if (!ingot)
      cas[i]->bead->covering_radius_rad =
        necklace->shape->ComputeCoveringSize(cas[i]->valid, cas[i]->bead->radius_base * scale) + bufferSize;
    else cas[i]->bead->covering_radius_rad = scale + bufferSize;
  }
  for (int i = 0; i < slices.size(); i++)
  {
    for (int j = 0; j < K; j++)
    {
      if (slices[i].tasks[j] != nullptr)
        slices[i].tasks[j]->radius_cur = slices[i].tasks[j]->bead->covering_radius_rad;
    }
  }

  // setup DP array
  int nSubSets = (1 << K);
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
    if (slices[i].eventLeft.type == TaskEvent::Type::kFrom)
    {
      int q = (1 << slices[i].eventLeft.node->layer);
      for (int j = 0; j < slices[i].sets.size(); j++)
      {
        int q2 = slices[i].sets[j];
        if ((q2 & q) > 0)
        {

          // split the circle (ranges, event times, the works)
          splitCircle(slices, K, i, q2);

          // compute
          bool good = feasibleLine(slices, K, opt, i, q2);

          if (good) return true;
        }
      }
    }
  }

  return false;
}

void Optimizer::splitCircle(std::vector<TaskSlice>& slices, const int K, const int slice, const BitString& split)
{
  // reset everything, then rotate
  for (int i = 0; i < slices.size(); i++)
  {
    slices[i].reset();
    slices[i].rotate(slices[slice].eventLeft.angle_rad, slices[slice].tasks, split);
  }
}

bool Optimizer::feasibleLine
(
  const std::vector<TaskSlice>& slices,
  const int K,
  std::vector<std::vector<OptValue> >& opt,
  const int slice,
  const int split
)
{
  int split2 = split ^(slices[slice].sets.back());  //TODO(tvl) rename split_inverse?

  // initialization
  //const TaskSlice& ts = slices[slice];
  opt[0][0].angle_rad = 0.0;
  opt[0][0].layer = -1;
  opt[0][0].cd = std::make_shared<CountryData>(nullptr, -1);

  for (int i = 0; i < slices.size(); i++)
  {
    int s = (slice + i) % slices.size();
    const TaskSlice& ts = slices[s];
    for (int j = 0; j < ts.sets.size(); j++)
    {
      int q = ts.sets[j];
      if (i == 0 && q == 0) continue;

      opt[i][q].angle_rad = std::numeric_limits<double>::max();
      opt[i][q].layer = -1;
      opt[i][q].cd = nullptr;

      if (i == 0 && (q & split2) > 0) continue;
      if (i == slices.size() - 1 && (q & split) > 0) continue;

      if (i != 0)
      {
        // check previous slice
        if (ts.eventLeft.type == TaskEvent::Type::kFrom)
        {
          if ((q & (1 << ts.eventLeft.layer)) == 0)
          {
            opt[i][q].angle_rad = opt[i - 1][q].angle_rad;
            opt[i][q].layer = opt[i - 1][q].layer;
            opt[i][q].cd = opt[i - 1][q].cd;
          }
        } else
        {
          int q2 = q + (1 << ts.eventLeft.node->layer);
          if (slices[(s + slices.size() - 1) % slices.size()].tasks[ts.eventLeft.node->layer]->disabled)
            q2 -= (1 << ts.eventLeft.node->layer); // special case
          opt[i][q].angle_rad = opt[i - 1][q2].angle_rad;
          opt[i][q].layer = opt[i - 1][q2].layer;
          opt[i][q].cd = opt[i - 1][q2].cd;
        }
      }
      if (opt[i][q].angle_rad < std::numeric_limits<double>::max()) continue;

      for (int x = 0; x < ts.taskCount; x++)
      {
        int k = ts.layers[x];
        CountryData::Ptr cd = ts.tasks[k];
        int k2 = (1 << k);
        if ((k2 & q) == 0) continue;
        if (cd->disabled) continue;

        double t1 = opt[i][q - k2].angle_rad;
        if (t1 == std::numeric_limits<double>::max()) continue;
        // special check
        if (opt[i][q - k2].cd->radius_cur == 0.0)
        {
          if (k != slices[slice].eventLeft.node->layer) continue;
        } else t1 += cd->radius_cur;

        t1 = std::max(t1, cd->range_cur->from());
        if (t1 <= cd->range_cur->to() && t1 + cd->radius_cur < opt[i][q].angle_rad)
        {
          opt[i][q].angle_rad = t1 + cd->radius_cur;
          opt[i][q].layer = k;
          opt[i][q].cd = cd;
        }
      }

    }
  }


  const TaskSlice& ts = slices[slice];
  if (opt[slices.size() - 1][split2].angle_rad == std::numeric_limits<double>::max()) return false;
  if (opt[slices.size() - 1][split2].angle_rad <= M_2xPI - ts.tasks[ts.eventLeft.node->layer]->radius_cur)
  {
    // feasible! construct solution
    int s = slices.size() - 1;
    int s2 = (slice + s) % slices.size();
    int q = split2;
    double t = opt[s][q].angle_rad - opt[s][q].cd->radius_cur;

    while (slices[s2].left > t + EPSILON)
    {
      if (slices[s2].eventLeft.type == TaskEvent::Type::kTo)
      {
        q += (1 << slices[s2].eventLeft.node->layer);
        if (s > 0 && slices[(s2 + slices.size() - 1) % slices.size()].tasks[slices[s2].eventLeft.node->layer]->disabled)
          q -= (1 << slices[s2].eventLeft.node->layer);
      }
      s--;
      s2 = (slice + s) % slices.size();
      if (s < 0) break;
    }


    while (s >= 0 && opt[s][q].layer != -1)
    {
      //System.out.println(s + ", " + q);
      CountryData::Ptr cd = opt[s][q].cd;
      if ((q & (1 << opt[s][q].layer)) == 0) return false;
      q -= (1 << opt[s][q].layer);
      cd->bead->angle_rad = t + slices[slice].eventLeft.angle_rad;
      t = opt[s][q].angle_rad - opt[s][q].cd->radius_cur;
      while (slices[s2].left > t + EPSILON)
      {
        if (slices[s2].eventLeft.type == TaskEvent::Type::kTo)
        {
          q += (1 << slices[s2].eventLeft.node->layer);
          if (s > 0 && slices[(s2 + slices.size() - 1) % slices.size()].tasks[slices[s2].eventLeft.node->layer]->disabled)
            q -= (1 << slices[s2].eventLeft.node->layer);
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

bool Optimizer::feasible2
(
  const std::vector<TaskSlice>& slices,
  const double scale,
  const int K,
  const double bufferSize,
  const Necklace::Ptr& necklace,
  const int copies,
  const bool ingot
)
{
  // set sizes
  for (int i = 0; i < cas.size(); i++)
  {
    if (!ingot)
      cas[i]->bead->covering_radius_rad =
        necklace->shape->ComputeCoveringSize(cas[i]->valid, cas[i]->bead->radius_base * scale) + bufferSize;
    else cas[i]->bead->covering_radius_rad = scale + bufferSize;
  }
  for (int i = 0; i < slices.size(); i++)
  {
    for (int j = 0; j < K; j++)
    {
      if (slices[i].tasks[j] != nullptr)
        slices[i].tasks[j]->radius_cur = slices[i].tasks[j]->bead->covering_radius_rad;
    }
  }

  // make new slices
  std::vector<TaskSlice> slices2(slices.size() * copies);
  for (int i = 0; i < copies; i++)
  {
    for (int j = 0; j < slices.size(); j++)
    {
      int q = i * slices.size() + j;
      slices2[q] = TaskSlice(slices[j], slices[0].left, i);
    }
  }

  // setup DP array
  int nSubSets = (1 << K);
  std::vector<std::vector<OptValue> > opt(slices2.size());
  for (int i = 0; i < slices2.size(); i++)
  {
    opt[i].resize(nSubSets);
    // TODO(tvl) check whether default initialization is done correctly...
    /*for (int j = 0; j < nSubSets; j++) {
      opt[i][j].Initialize();
    }*/
  }

  return feasibleLine2(slices2, K, opt, false);
}

bool Optimizer::feasibleLine2
(
  const std::vector<TaskSlice>& slices,
  const int K,
  std::vector<std::vector<OptValue> >& opt,
  const bool lookup/*note, not used in practice*/
)
{
  // initialization
  //const TaskSlice& ts = slices[0];
  opt[0][0].angle_rad = 0.0;
  opt[0][0].angle2_rad = 0.0;
  opt[0][0].layer = -1;
  opt[0][0].cd = std::make_shared<CountryData>(nullptr, -1);

  for (int i = 0; i < slices.size(); i++)
  {
    const TaskSlice& ts = slices[i];
    for (int j = 0; j < ts.sets.size(); j++)
    {
      int q = ts.sets[j];
      if (i == 0 && q == 0) continue;

      opt[i][q].angle_rad = std::numeric_limits<double>::max();
      opt[i][q].angle2_rad = opt[i][q].angle_rad;
      opt[i][q].layer = -1;
      opt[i][q].cd = nullptr;

      if (i != 0)
      {
        // check previous slice
        if (ts.eventLeft.type == TaskEvent::Type::kFrom)
        {
          if ((q & (1 << ts.eventLeft.node->layer)) == 0)
          {
            opt[i][q].angle_rad = opt[i - 1][q].angle_rad;
            opt[i][q].angle2_rad = opt[i - 1][q].angle2_rad;
            opt[i][q].layer = opt[i - 1][q].layer;
            opt[i][q].cd = opt[i - 1][q].cd;
          }
        } else
        {
          int q2 = q + (1 << ts.eventLeft.node->layer);
          if (slices[i - 1].tasks[ts.eventLeft.node->layer] == nullptr) q2 -= (1 << ts.eventLeft.node->layer); // special case
          opt[i][q].angle_rad = opt[i - 1][q2].angle_rad;
          opt[i][q].angle2_rad = opt[i - 1][q2].angle2_rad;
          opt[i][q].layer = opt[i - 1][q2].layer;
          opt[i][q].cd = opt[i - 1][q2].cd;
        }
      }

      if (opt[i][q].angle_rad < std::numeric_limits<double>::max()) continue;

      for (int x = 0; x < ts.taskCount; x++)
      {
        int k = ts.layers[x];
        CountryData::Ptr cd = ts.tasks[k];
        int k2 = (1 << k);
        if ((k2 & q) == 0) continue;
        if (cd == nullptr) continue;

        double t1 = opt[i][q - k2].angle_rad;
        if (t1 == std::numeric_limits<double>::max()) continue;
        // lookup size if spline
        double size = cd->radius_cur;
        // special check
        if (opt[i][q - k2].cd->radius_cur != 0.0)
        {
          if (lookup)
          {
            double angle = t1 + size;
            for (int z = 0; z < 5; z++)
            {
              size = cd->lookUpSize(angle);
              angle = t1 + size;
            }
            t1 = angle;
          } else t1 += size;
        } else if (lookup) size = cd->lookUpSize(t1);

        t1 = std::max(t1, cd->range_cur->from());
        if (lookup) size = cd->lookUpSize(t1);
        if (t1 <= cd->range_cur->to() && t1 + size < opt[i][q].angle_rad)
        {
          opt[i][q].angle_rad = t1 + size;
          opt[i][q].angle2_rad = t1;
          opt[i][q].layer = k;
          opt[i][q].cd = cd;
        }
      }

    }
  }


  CountryAngleSet listCA;

  int s = slices.size() - 1;
  int q = slices[s].sets[slices[s].sets.size() - 1];
  double t = opt[s][q].angle_rad;
  if (t == std::numeric_limits<double>::max()) return false;
  //t -= opt[s][q].cd.size;
  t = opt[s][q].angle2_rad;

  while (slices[s].left > t + EPSILON)
  {
    if (slices[s].eventLeft.type == TaskEvent::Type::kTo)
    {
      q += (1 << slices[s].eventLeft.node->layer);
      if (s > 0 && slices[s - 1].tasks[slices[s].eventLeft.node->layer] == nullptr) q -= (1 << slices[s].eventLeft.node->layer);
    }
    s--;
    if (s < 0) break;
  }

  while (s >= 0 && opt[s][q].layer != -1)
  {
    CountryData::Ptr cd = opt[s][q].cd;
    q -= (1 << opt[s][q].layer);
    if (q < 0 || cd == nullptr) return false;
    double size = cd->radius_cur;
    if (lookup) size = cd->lookUpSize(t);
    listCA.push_back(std::make_shared<AnyOrderCycleNode>(cd->bead, t + slices[0].eventLeft.angle_rad, size));
    t = opt[s][q].angle2_rad;
    while (slices[s].left > t + EPSILON)
    {
      if (slices[s].eventLeft.type == TaskEvent::Type::kTo)
      {
        q += (1 << slices[s].eventLeft.node->layer);
        if (s > 0 && slices[s - 1].tasks[slices[s].eventLeft.node->layer] == nullptr) q -= (1 << slices[s].eventLeft.node->layer);
      }
      s--;
      if (s < 0) break;
    }
  }


  // set to not found
  int count = 0;
  for (int i = 0; i < cas.size(); i++)
    cas[i]->bead->check = 0;

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
    if (count == cas.size()) break;
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

  if (count == cas.size())
    return true;

  return false;
}

} // namespace detail
} // namespace necklace_map
} // namespace geoviz
