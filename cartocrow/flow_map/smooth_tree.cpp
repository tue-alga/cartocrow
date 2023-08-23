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

SmoothTree::SmoothTree(const std::shared_ptr<SpiralTree> spiralTree) : m_tree(spiralTree) {
	Number<Inexact> rMax = 0;
	for (const auto& node : m_tree->nodes()) {
		if (node->m_position.r() > rMax) {
			rMax = node->m_position.r();
		}
	}
	constructSmoothTree(spiralTree->root(), rMax / 10); // TODO make 10 configurable
}

std::shared_ptr<Node> SmoothTree::constructSmoothTree(const std::shared_ptr<Node>& node, Number<Inexact> maxRStep) {
	auto smoothNode = std::make_shared<Node>(node->m_position);
	m_nodes.push_back(smoothNode);
	for (const auto& child : node->m_children) {
		auto smoothChild = constructSmoothTree(child, maxRStep);
		const Spiral spiral(node->m_position, child->m_position);
		Number<Inexact> rMin = node->m_position.r();
		Number<Inexact> rMax = child->m_position.r();
		Number<Inexact> rRange = rMax - rMin;
		int subdivisionCount = std::ceil(rRange / maxRStep);
		auto previous = smoothNode;
		for (double i = 1; i < subdivisionCount; ++i) {
			const PolarPoint position =
			    spiral.evaluate(spiral.parameterForR(rMin + i * (rMax - rMin) / subdivisionCount));
			auto subdivision = std::make_shared<Node>(position);
			m_nodes.push_back(subdivision);
			subdivision->m_parent = previous;
			previous->m_children.push_back(subdivision);
			previous = subdivision;
		}
		smoothChild->m_parent = previous;
		previous->m_children.push_back(smoothChild->m_parent);
	}

	return smoothNode;
}

const std::vector<std::shared_ptr<Node>>& SmoothTree::nodes() const {
	return m_nodes;
}

} // namespace cartocrow::flow_map
