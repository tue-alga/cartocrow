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
*/

#ifndef CARTOCROW_FLOW_MAP_SMOOTH_TREE_H
#define CARTOCROW_FLOW_MAP_SMOOTH_TREE_H

#include <deque>
#include <map>
#include <memory>
#include <queue>
#include <vector>

#include "spiral_tree.h"

namespace cartocrow::flow_map {

/// A smoothed tree.
class SmoothTree {

  public:
	/// Constructs a smooth tree.
	SmoothTree(const std::shared_ptr<SpiralTree> spiralTree);

	/// Returns a list of the nodes in this smooth tree.
	const std::vector<std::shared_ptr<Node>>& nodes() const;

	/// Performs one optimization step.
	void optimize();

  //private:  // TODO temporary
	/// The spiral tree underlying this smooth tree.
	std::shared_ptr<SpiralTree> m_tree;

	/// List of nodes in this tree.
	std::vector<std::shared_ptr<Node>> m_nodes;

	std::shared_ptr<Node> constructSmoothTree(const std::shared_ptr<Node>& node, Number<Inexact> maxRStep);

	Number<Inexact> computeSmoothFunction(const std::shared_ptr<Node>& node);
	Number<Inexact> computeSmoothForce(const std::shared_ptr<Node>& node);
};

} // namespace cartocrow::flow_map

#endif
