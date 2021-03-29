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

#ifndef GEOVIZ_NECKLACE_MAP_DETAIL_CHECK_FEASIBLE_HEURISTIC_H
#define GEOVIZ_NECKLACE_MAP_DETAIL_CHECK_FEASIBLE_HEURISTIC_H

#include "check_feasible.h"

#include <memory>
#include <vector>

#include "geoviz/common/range.h"
#include "geoviz/necklace_map/bead.h"
#include "geoviz/necklace_map/detail/cycle_node.h"
#include "geoviz/necklace_map/detail/cycle_node_layered.h"


namespace geoviz
{
namespace necklace_map
{
namespace detail
{

// A cycle node that can be assigned a layer.
struct CycleNodeCheck : public CycleNode
{
  using Ptr = std::shared_ptr<CycleNodeCheck>;

  CycleNodeCheck(const Bead::Ptr& bead, const Number& angle_rad);

  int check;
  Number angle_rad;
}; // struct CycleNodeCheck


// The heuristic algorithm for the feasibility decision problem computes a number of node orderings hoping to find a valid placement.
// This takes O(n*log(n) + cnK2^K) time, where n is the number of nodes, c is the number of heuristic steps (typically 5), and K is the 'width' of the node set (i.e. the maximum number of valid intervals intersected by a ray originating for the necklace kernel).
class CheckFeasibleHeuristic : public CheckFeasible
{
 public:
  CheckFeasibleHeuristic(NodeSet& nodes, const int heuristic_cycles);

  bool operator()() override;

 private:
  void InitializeSlices() override;

  void AssignAngle(const Number& angle_rad, Bead::Ptr& bead) override;

  bool Feasible();

  const int heuristic_cycles_;

  using CheckSet = std::vector<CycleNodeCheck::Ptr>;
  CheckSet nodes_check_;
}; // class CheckFeasibleHeuristic

} // namespace detail
} // namespace necklace_map
} // namespace geoviz

#endif //GEOVIZ_NECKLACE_MAP_DETAIL_CHECK_FEASIBLE_HEURISTIC_H
