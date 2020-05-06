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

#include "cycle_node_layered.h"


namespace geoviz
{
namespace necklace_map
{
namespace detail
{

CycleNodeLayered::CycleNodeLayered() :
  CycleNode(), layer(-1), disabled(true)
{}

CycleNodeLayered::CycleNodeLayered(const Bead::Ptr& bead) :
  CycleNode(bead), layer(-1), disabled(!bead)
{}

CycleNodeLayered::CycleNodeLayered(const CycleNodeLayered::Ptr& node) :
  CycleNode(), layer(-1), disabled(!node)
{
  if (node)
  {
    bead = node->bead;
    valid = node->valid;
    layer = node->layer;
  }
}

bool CompareCycleNodeLayered::operator()(const CycleNodeLayered::Ptr& a, const CycleNodeLayered::Ptr& b) const
{
  return a->valid->from() < b->valid->from();
}

} // namespace detail
} // namespace necklace_map
} // namespace geoviz
