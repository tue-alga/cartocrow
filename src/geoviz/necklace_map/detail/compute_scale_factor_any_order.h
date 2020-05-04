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

  AnyOrderCycleNode();

  explicit AnyOrderCycleNode(const Bead::Ptr& bead);

  AnyOrderCycleNode(const AnyOrderCycleNode::Ptr& node);

  int layer;
  bool disabled;
  int check;
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

  TaskEvent(const AnyOrderCycleNode::Ptr& node, const Number& angle_rad, const Type& type);

  AnyOrderCycleNode::Ptr node;
  Number angle_rad;
  Type type;
}; // class TaskEvent

class CompareTaskEvent
{
 public:
  bool operator()(const TaskEvent& a, const TaskEvent& b) const;
}; // class CompareTaskEvent


class BitString
{
 public:
  inline static bool CheckFit(const int bit) { return bit < 32; }

  inline static BitString FromBit(const int bit) { return BitString(ToString(bit)); }
  inline static BitString FromString(const int bits) { return BitString(bits); }

  BitString() : bits(0) {}

  inline bool Empty() const { return bits == 0; }

  inline bool HasBit(const int bit) const { return (ToString(bit) & bits) != 0; }

  inline bool Overlaps(const BitString& string) const { return (string.bits & bits) != 0; }

  inline const int& Get() const { return bits; }

  inline void AddBit(const int bit) { bits |= ToString(bit); }

  inline void RemoveBit(const int bit) { bits &= ~ToString(bit); }

  inline BitString operator^(const BitString& string) const { return BitString(bits ^ string.bits); }

  inline BitString PlusBit(const int bit) const { return BitString(bits | ToString(bit)); }

  inline BitString MinusBit(const int bit) const { return BitString(bits & ~ToString(bit)); }

 private:
  inline static int ToString(const int bit) { return 1 << bit; }

  explicit BitString(const int string) : bits(string) {}

  int bits;
}; // class BitString


class TaskSlice
{
 public:
  TaskSlice();

  TaskSlice
  (
    const TaskEvent& event_from,
    const TaskEvent& event_to,
    const int num_layers
  );

  TaskSlice(const TaskSlice& slice, const int cycle);

  void Reset();

  void Rotate(const TaskSlice& first_slice, const BitString& layer_set);

  void AddTask(const AnyOrderCycleNode::Ptr& task);

  void Finalize();

  TaskEvent event_from, event_to;
  Range coverage;

  std::vector<AnyOrderCycleNode::Ptr> tasks;
  std::vector<BitString> layer_sets;
}; // class TaskSlice


class CheckFeasible
{
 public:
  using NodeSet = std::vector<AnyOrderCycleNode::Ptr>;
  using Ptr = std::shared_ptr<CheckFeasible>;

  static Ptr New(NodeSet& nodes, const int heuristic_cycles);

  void Initialize();

  // Note that the covering radius of each node should be set before calling this.
  virtual bool operator()() = 0;

 protected:
  struct Value
  {
    Value();

    void Reset();

    Number CoveringRadius() const;

    AnyOrderCycleNode::Ptr task;
    Number angle_rad;
  }; // struct Value

  CheckFeasible(NodeSet& nodes);

  virtual void InitializeSlices();

  void InitializeContainer();

  void ResetContainer();

  void ComputeValues
  (
    const size_t slice_index_offset,
    const BitString& first_layer_set,
    const BitString& first_unused_set
  );

  virtual void ProcessTask(const AnyOrderCycleNode::Ptr&) {}

  bool AssignAngles
  (
    const size_t slice_index_offset,
    const BitString& unused_set
  );

  NodeSet& nodes_;
  std::vector<TaskSlice> slices_;
  std::vector<std::vector<Value> > values_;
}; // class CheckFeasible


class CheckFeasibleExact : public CheckFeasible
{
 public:
  CheckFeasibleExact(NodeSet& nodes);

  bool operator()() override;

 private:
  void SplitCircle
  (
    const TaskSlice& first_slice,
    const BitString& layer_set
  );

  bool FeasibleFromSlice
  (
    const size_t first_slice_index,
    const BitString& first_layer_set
  );
}; // class CheckFeasibleExact


class CheckFeasibleHeuristic : public CheckFeasible
{
 public:
  CheckFeasibleHeuristic(NodeSet& nodes, const int heuristic_cycles);

  bool operator()() override;

 private:
  void InitializeSlices() override;

  void ProcessTask(const AnyOrderCycleNode::Ptr& task);

  bool Feasible();

  const int heuristic_cycles_;

  NodeSet nodes_check_;
}; // class CheckFeasibleHeuristic


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
    const int heuristic_cycles = 5
  );

  Number Optimize();

 protected:
  virtual Number ComputeScaleUpperBound() const;

  virtual Number ComputeCoveringRadii(const Number& scale_factor);

 private:
  int AssignLayers();

 protected:
  NecklaceShape::Ptr necklace_shape_;

  // Note that the scaler must be able to access the set by index.
  NodeSet nodes_;

  Number half_buffer_rad_;
  //Number max_buffer_rad_;  // Based on smallest scaled radius?

  int binary_search_depth_;
  CheckFeasible::Ptr check_;
}; // class ComputeScaleFactorAnyOrder


class ComputeScaleFactorAnyOrderIngot : public ComputeScaleFactorAnyOrder
{
 public:
  ComputeScaleFactorAnyOrderIngot
  (
    const Necklace::Ptr& necklace,
    const Number& buffer_rad = 0,
    const int binary_search_depth = 10,
    const int heuristic_cycles = 5
  );

 protected:
  Number ComputeScaleUpperBound() const override;

  Number ComputeCoveringRadii(const Number& scale_factor) override;
}; // class ComputeScaleFactorAnyOrderIngot

} // namespace detail
} // namespace necklace_map
} // namespace geoviz

#endif //GEOVIZ_NECKLACE_MAP_DETAIL_COMPUTE_SCALE_FACTOR_ANY_ORDER_H
