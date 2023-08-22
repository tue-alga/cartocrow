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

#include "smooth_tree.h"
#include "cartocrow/flow_map/smooth_tree_painting.h"

namespace cartocrow::flow_map {

SmoothTree::SmoothTree(const std::shared_ptr<SpiralTree> spiralTree) : m_tree(spiralTree) {}

const std::vector<std::shared_ptr<Node>>& SmoothTree::nodes() const {
	return m_nodes;
}

} // namespace cartocrow::flow_map
