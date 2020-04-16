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

// A cycle node that can be assigned to a layer.
// The cycle nodes on a single layer shall never have valid intervals that intersect in their interior.
struct AnyOrderCycleNode : public CycleNode
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


// The cycle nodes will be processed by moving from event to event.
// Each event indicates that a valid interval starts or stops at the associated angle.
struct TaskEvent
{
  enum class Type { kFrom, kTo };

  TaskEvent();

  TaskEvent(AnyOrderCycleNode::Ptr& node, const Number& angle_rad, const Type& type);

  AnyOrderCycleNode::Ptr node;
  Number angle_rad;
  Type type;
}; // class TaskEvent

class CompareTaskEvent
{
 public:
  bool operator()(const TaskEvent& a, const TaskEvent& b) const;
}; // class CompareTaskEvent





// the bead as it is stored by the 'taskslice'.
struct BeadData  // TODO(tvl) replace by AnyOrderCycleNode (and forward members)?
{
  using Ptr = std::shared_ptr<BeadData>;

  BeadData(const Bead::Ptr& bead, const int layer);

  BeadData(const BeadData& data);

  Bead::Ptr bead;
  Range::Ptr valid;  // TODO(tvl) check whether this can be const?
  int layer;
  bool disabled;  // TODO(tvl) replace by check for bead pointer validity?
}; // struct BeadData






// The bit strings are used to keep track of the layers that are 'active' within a certain angle interval.
class BitString
{
 public:
  static bool CheckFit(const int bit);

  BitString() : bits(0) {}

  explicit BitString(const int string) : bits(string) {}

  inline const int& Get() const { return bits; }

  inline bool IsEmpty() const { return bits == 0; }

  inline bool HasBit(const int bit) const { return (1 << bit) & bits != 0; }

  inline bool HasAny(const BitString& string) const { return (string.bits & bits) != 0; }

  void SetBit(const int bit);

  inline void SetString(const int string) { bits = string; }

  bool AddBit(const int bit);

  inline BitString Xor(const BitString& string) const { return BitString(bits ^ string.bits); }

 private:
  int bits;
}; // class BitString







class TaskSlice
{
 public:
  TaskSlice();

  TaskSlice(const TaskEvent& eLeft, const TaskEvent& eRight, const int K, const double right);

  TaskSlice(const TaskSlice& ts, const double offset, const int step);

  void reset();

  void rotate(const double value, const std::vector<BeadData::Ptr>& cds, const BitString& split);

  void addTask(const BeadData::Ptr& task);

  void produceSets();

  TaskEvent eventLeft, eventRight;
  std::vector<BeadData::Ptr> tasks;
  int taskCount;
  double left, right;  // TODO(tvl) should this be left_time, right_time?
  std::vector<BitString> sets;
  std::vector<int> layers;
}; // class TaskSlice


struct OptValue
{
  OptValue();

  void Initialize();

  double angle_rad;
  double angle2_rad;
  int layer;
  BeadData::Ptr cd;
}; // struct OptValue


class ComputeScaleFactorAnyOrder
{
 protected:
  using NodeSet = std::vector<AnyOrderCycleNode::Ptr>;

 public:
  ComputeScaleFactorAnyOrder
  (
    const Necklace::Ptr& necklace,
    const Number& buffer_rad = 0,
    const int binary_search_depth = 10,
    const int heuristic_steps = 5
  );

  Number Optimize();

 protected:
  virtual Number ComputeScaleUpperBound() const;

  virtual Number ComputeCoveringRadii(const Number& scale_factor);

 private:







  bool feasible
  (
    std::vector<TaskSlice>& slices,
    const int K
  );

  void splitCircle
  (
    std::vector<TaskSlice>& slices,
    const int slice,
    const BitString& split
  );

  bool feasibleLine
  (
    const std::vector<TaskSlice>& slices,
    std::vector<std::vector<OptValue> >& opt,
    const int slice,
    const BitString& split
  );

  bool feasible2
  (
    const std::vector<TaskSlice>& slices,
    const int K,
    const int copies
  );

  bool feasibleLine2
  (
    const std::vector<TaskSlice>& slices,
    std::vector<std::vector<OptValue> >& opt
  );

 protected:
  NecklaceShape::Ptr necklace_shape_;

  // Note that the scaler must be able to access the set by index.
  NodeSet nodes_;

  Number necklace_length_;
  Number half_buffer_rad_;
  //Number max_buffer_rad_;  // Based on smallest scaled radius?

  int binary_search_depth_;
  int heuristic_steps_; // TODO(tvl) const value: remove? Make separate class/struct hierarchy?
}; // class ComputeScaleFactorAnyOrder

class ComputeScaleFactorAnyOrderIngot : public ComputeScaleFactorAnyOrder
{
 public:
  ComputeScaleFactorAnyOrderIngot
  (
    const Necklace::Ptr& necklace,
    const Number& buffer_rad = 0,
    const int binary_search_depth = 10,
    const int heuristic_steps = 5
  );

 protected:
  Number ComputeScaleUpperBound() const override;

  Number ComputeCoveringRadii(const Number& scale_factor) override;
}; // class ComputeScaleFactorAnyOrderIngot

} // namespace detail
} // namespace necklace_map
} // namespace geoviz

#endif //GEOVIZ_NECKLACE_MAP_DETAIL_COMPUTE_SCALE_FACTOR_ANY_ORDER_H
