/*
The Flow Map library implements the algorithmic geo-visualization
method by the same name, developed by Kevin Verbeek, Kevin Buchin,
and Bettina Speckmann at TU Eindhoven
(DOI: 10.1007/s00453-013-9867-z & 10.1109/TVCG.2011.202).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 16-10-2020
*/

#include "flow_tree.h"

#include <glog/logging.h>


namespace geoviz
{
namespace flow_map
{

FlowTree::Node::Node(const Type type, Place::Ptr place, const PolarPoint& relative_position) :
  type(type),
  place(place),
  relative_position(relative_position)
{}

// Flow tree is the thickened tree, as opposed to the 'thin' spiral tree.
// Note that while the spiral tree is a binary tree, the flow tree is not necessarily a binary tree.

FlowTree::FlowTree(const SpiralTree& spiral_tree) :
  root_(spiral_tree.root_)
{
  for (const SpiralTree::NodePtr& node : spiral_tree.nodes_)
  {
    nodes_.emplace_back(node->type, node->place, node->relative_position);

    if (node->arc_parent != nullptr)
    {
      const SpiralTree::ArcPtr& arc = node->arc_parent;

      Number angle_rad = 0;
      switch (arc->side)
      {
        case SpiralTree::Arc::Side::kLeft:
          angle_rad = -spiral_tree.restricting_angle_rad_;
          break;
        case SpiralTree::Arc::Side::kRight:
          angle_rad = spiral_tree.restricting_angle_rad_;
          break;
      }
      const Spiral spiral(angle_rad, arc->child->relative_position);

      arcs_.emplace_back(spiral, arc->parent->relative_position);
    }
  }
}

} // namespace flow_map
} // namespace geoviz
