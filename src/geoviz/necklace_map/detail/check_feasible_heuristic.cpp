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

#include "check_feasible_heuristic.h"


namespace geoviz
{
namespace necklace_map
{
namespace detail
{

CycleNodeCheck::CycleNodeCheck(const Bead::Ptr& bead, const Range::Ptr& valid) :
  CycleNode(bead, valid), check(0)
{}


CheckFeasibleHeuristic::CheckFeasibleHeuristic(NodeSet& nodes, const int heuristic_cycles) :
  CheckFeasible(nodes),
  heuristic_cycles_(heuristic_cycles)
{}

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
  std::vector<TaskSlice> slices_clone;
  slices_clone.swap(slices_);

  const size_t num_slices = slices_clone.size();
  slices_.resize(num_slices * heuristic_cycles_);
  for (int cycle = 0; cycle < heuristic_cycles_; ++cycle)
    for (size_t j = 0; j < num_slices; ++j)
      slices_[cycle * num_slices + j] = TaskSlice(slices_clone[j], cycle);
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

} // namespace detail
} // namespace necklace_map
} // namespace geoviz
