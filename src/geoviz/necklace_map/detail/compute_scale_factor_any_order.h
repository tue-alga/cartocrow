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

#ifndef GEOVIZ_NECKLACE_MAP_DETAIL_COMPUTE_SCALE_FACTOR_ANY_ORDER_H
#define GEOVIZ_NECKLACE_MAP_DETAIL_COMPUTE_SCALE_FACTOR_ANY_ORDER_H

#include <memory>
#include <vector>

#include "geoviz/common/core_types.h"
#include "geoviz/necklace_map/bead.h"
#include "geoviz/necklace_map/necklace.h"
#include "geoviz/necklace_map/range.h"
#include "geoviz/necklace_map/detail/cycle_node.h"


// TODO(tvl) if ever moving to C++17, implement nested namespace definition "namespace geoviz::necklace_map {"
namespace geoviz
{
namespace necklace_map
{
namespace detail
{

struct AnyOrderCycleNode : CycleNode
{
  using Ptr = std::shared_ptr<AnyOrderCycleNode>;

  AnyOrderCycleNode(const Bead::Ptr& bead);

  AnyOrderCycleNode(const Bead::Ptr& bead, const Number& angle_rad, const Number& buffer_rad);

  int layer;
}; // struct AnyOrderCycleNode

class CompareAnyOrderCycleNode
{
 public:
  bool operator()(const AnyOrderCycleNode::Ptr& a, const AnyOrderCycleNode::Ptr& b) const;
}; // class CompareAnyOrderCycleNode


struct TaskEvent
{
  TaskEvent();

  TaskEvent(const int layer, const double time, const bool is_start, const size_t index_node);

  int layer;
  double time; // TODO(tvl) rename "order"?
  bool is_start;  // Whether this is a "start feasible interval" event.
  size_t index_node;  // index of the country angle.
}; // class TaskEvent

class CompareTaskEvent
{
 public:
  bool operator()(const TaskEvent& a, const TaskEvent& b) const;
}; // class CompareTaskEvent

struct CountryData  // TODO(tvl) rename "BeadData"?
{
  using Ptr = std::shared_ptr<CountryData>;

  CountryData(const Bead::Ptr& bead, const int layer);

  CountryData(const CountryData& cd);

  double lookUpSize(double angle); // TODO(tvl) remove; replace by just querying radius_cur.

  Bead::Ptr bead;
  Range::Ptr range_cur; // TODO(tvl) rename "valid"?
  Number radius_cur;  // TODO(tvl) check whether this is actually used or could just forward to bead->covering_radius_rad
  int layer;
  bool disabled;  // TODO(tvl) replace by check for bead pointer validity?
}; // struct CountryData


class TaskSlice
{
 public:
  TaskSlice();

  TaskSlice(const TaskEvent& eLeft, const TaskEvent& eRight, const int K, const double right);

  TaskSlice(const TaskSlice& ts, const double offset, const int step);

  void reset();

  void rotate(const double value, const std::vector<CountryData::Ptr>& cds, const int split);

  void addTask(const CountryData::Ptr& task);

  void produceSets();

  TaskEvent eventLeft, eventRight;
  std::vector<CountryData::Ptr> tasks;
  int taskCount;
  double left, right;  // TODO(tvl) should this be left_time, right_time?
  std::vector<int> sets;
  std::vector<int> layers;
}; // class TaskSlice


struct OptValue
{
  OptValue();

  void Initialize();

  double time;
  double time2;
  int layer;
  CountryData::Ptr cd;
}; // struct OptValue


class Optimizer
{
 public:
  double computeOptSize
  (
    const double bufferSize/*rename buffer_rad*/,
    const int precision,
    const Necklace::Ptr& cg/*rename: necklace*/,
    const int heurSteps, // heurSteps == 0 -> Exact algo
    const bool ingot  // TODO(tvl) remove.
  );

  bool feasible
  (
    std::vector<TaskSlice>& slices,
    const double scale,
    const int K,
    const double bufferSize,
    const Necklace::Ptr& necklace,
    const bool ingot
  );

  void splitCircle(std::vector<TaskSlice>& slices, const int K, const int slice, const int split);

  bool feasibleLine
  (
    const std::vector<TaskSlice>& slices,
    const int K,
    std::vector<std::vector<OptValue> >& opt,
    const int slice,
    const int split
  );

  bool feasible2
  (
    const std::vector<TaskSlice>& slices,
    const double scale,
    const int K,
    const double bufferSize,
    const Necklace::Ptr& necklace,
    const int copies,
    const bool ingot
  );

  bool feasibleLine2
  (
    const std::vector<TaskSlice>& slices,
    const int K,
    std::vector<std::vector<OptValue> >& opt,
    const bool lookup/*note, not used in practice*/
  );

 private:
  using CountryAngleSet = std::vector<AnyOrderCycleNode::Ptr>;
  CountryAngleSet cas;  //TODO(tvl) move from class member to function parameter?
}; // class Optimizer

} // namespace detail
} // namespace necklace_map
} // namespace geoviz

#endif //GEOVIZ_NECKLACE_MAP_DETAIL_COMPUTE_SCALE_FACTOR_ANY_ORDER_H
