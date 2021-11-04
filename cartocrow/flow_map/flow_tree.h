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

#ifndef CARTOCROW_FLOW_MAP_FLOW_TREE_H
#define CARTOCROW_FLOW_MAP_FLOW_TREE_H

#include <memory>
#include <vector>

#include "cartocrow/core/spiral.h"
#include "cartocrow/flow_map/place.h"
#include "cartocrow/flow_map/spiral_tree.h"

namespace cartocrow::flow_map {

/// A tree where each arc is a smooth curve with a thickness indicating the
/// flow.
/**
 * This tree is based on a spiral tree. Unlike a spiral tree, a flow tree is
 * not necessarily a binary tree.
 */
class FlowTree {
  public:
	/// The preferred pointer type for storing or sharing a flow tree.
	using Ptr = std::shared_ptr<FlowTree>;

	/// The type for the initial arcs in the flow tree, before they are
	/// assigned a thickness or pushed farther from obstacles.
	// TODO(tvl) the second element should probably be the minimum R, instead
	// of a full point. Replace by SpiralSegment? Is this one even needed once
	// we add thickness to the arcs; replace by custom class?
	using FlowArc = std::pair<Spiral, PolarPoint>;

	/// Construct a flow tree.
	/**
	 * @param spiral_tree The spiral tree describing the initial arrangement of
	 * the arcs of the tree.
	 */
	FlowTree(const SpiralTree& spiral_tree);

	Vector root_translation_;

	std::vector<Node::Ptr> nodes_; // Note that the positions of these nodes are offset by the position of the root.

	std::vector<Region> obstacles_; // TODO(tvl) remove after debugging: output should use original obstacles, not adjusted obstacles.

	std::vector<FlowArc> arcs_;
}; // class FlowTree

} // namespace cartocrow::flow_map

#endif //CARTOCROW_FLOW_MAP_FLOW_TREE_H
