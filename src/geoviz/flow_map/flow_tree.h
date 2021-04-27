/*
The Flow Map library implements the algorithmic geo-visualization
method by the same name, developed by Kevin Verbeek, Kevin Buchin,
and Bettina Speckmann at TU Eindhoven
(DOI: 10.1007/s00453-013-9867-z & 10.1109/TVCG.2011.202).
Copyright (C) 2021  Netherlands eScience Center and TU Eindhoven

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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 16-10-2020
*/

#ifndef GEOVIZ_FLOW_MAP_FLOW_TREE_H
#define GEOVIZ_FLOW_MAP_FLOW_TREE_H

#include <memory>
#include <vector>

#include "geoviz/common/spiral.h"
#include "geoviz/flow_map/place.h"
#include "geoviz/flow_map/spiral_tree.h"


namespace geoviz
{
namespace flow_map
{

class FlowTree
{
 public:
  using Ptr = std::shared_ptr<FlowTree>;

  using FlowArc = std::pair<Spiral, PolarPoint>;  // TODO(tvl) the second element should probably be the minimum R, instead of a full point. Replace by SpiralSegment? Is this one even needed once we add thickness to the arcs; replace by custom class?

  FlowTree(const SpiralTree& spiral_tree);

 //private:
  Vector root_translation_;

  std::vector<Node::Ptr> nodes_;  // Note that the positions of these nodes are offset by the position of the root.

  std::vector<Region> obstacles_;  // TODO(tvl) remove after debugging: output should use original obstacles, not adjusted obstacles.

  std::vector<FlowArc> arcs_;
}; // class FlowTree

} // namespace flow_map
} // namespace geoviz

#endif //GEOVIZ_FLOW_MAP_FLOW_TREE_H
