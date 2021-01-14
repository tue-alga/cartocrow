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

// Flow tree is the thickened tree, as opposed to the 'thin' spiral tree.
// Note that while the spiral tree is a binary tree, the flow tree is not necessarily a binary tree.

FlowTree::FlowTree(const SpiralTree& spiral_tree) :
  root_translation_(Point(CGAL::ORIGIN) - spiral_tree.GetRoot()),
  nodes_(spiral_tree.nodes_begin(), spiral_tree.nodes_end())
{
  for (const Node::Ptr& node : nodes_)
  {
    if (node->parent == nullptr)
      continue;

    const PolarPoint node_relative_position(node->place->position, root_translation_);
    const PolarPoint parent_relative_position(node->parent->place->position, root_translation_);

    const Spiral spiral(node_relative_position, parent_relative_position);
    arcs_.emplace_back(spiral, parent_relative_position);
  }
}

} // namespace flow_map
} // namespace geoviz
