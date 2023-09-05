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
#include <limits>

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
	if (node->getType() == Node::ConnectionType::kLeaf) {
		smoothNode->m_flow = node->m_place->m_flow;
	} else {
		smoothNode->m_flow = 0;
		for (const auto& child : node->m_children) {
			auto smoothChild = constructSmoothTree(child, maxRStep);
			smoothNode->m_flow += smoothChild->m_flow;
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
				subdivision->m_flow = smoothChild->m_flow;
				m_nodes.push_back(subdivision);
				subdivision->m_parent = previous;
				previous->m_children.push_back(subdivision);
				previous = subdivision;
			}
			smoothChild->m_parent = previous;
			previous->m_children.push_back(smoothChild);
		}
	}

	return smoothNode;
}

const std::vector<std::shared_ptr<Node>>& SmoothTree::nodes() const {
	return m_nodes;
}

Number<Inexact> SmoothTree::computeSmoothingFunction(const std::shared_ptr<Node>& node) {
	PolarPoint n = node->m_position;
	PolarPoint p = node->m_parent->m_position;
	PolarPoint c = node->m_children[0]->m_position;
	return std::pow(Spiral::alpha(p, n) - Spiral::alpha(n, c), 2);
}

void SmoothTree::applySmoothingForce(int i, int iParent, int iChild) {
	PolarPoint n = m_nodes[i]->m_position;
	PolarPoint p = m_nodes[iParent]->m_position;
	PolarPoint c = m_nodes[iChild]->m_position;

	// chain rule: the derivative of (β_1 - β_2)² is 2(β_1 - β_2) * [β_1 - β_2]'
	m_forces[i].r += 2 * (Spiral::alpha(p, n) - Spiral::alpha(n, c)) *
	                 (Spiral::dAlphaDR2(p, n) - Spiral::dAlphaDR1(n, c));
	m_forces[i].phi += 2 * (Spiral::alpha(p, n) - Spiral::alpha(n, c)) *
	                   (Spiral::dAlphaDPhi2(p, n) - Spiral::dAlphaDPhi1(n, c));

	m_forces[iParent].r += 2 * (Spiral::alpha(p, n) - Spiral::alpha(n, c)) *
	                 (Spiral::dAlphaDR1(p, n));
	m_forces[iParent].phi += 2 * (Spiral::alpha(p, n) - Spiral::alpha(n, c)) *
	                   (Spiral::dAlphaDPhi1(p, n));

	m_forces[iChild].r += 2 * (Spiral::alpha(p, n) - Spiral::alpha(n, c)) *
	                 (Spiral::dAlphaDR2(n, c));
	m_forces[iChild].phi += 2 * (Spiral::alpha(p, n) - Spiral::alpha(n, c)) *
	                   (Spiral::dAlphaDPhi2(n, c));
}

void SmoothTree::optimize() {
	m_forces = std::vector<PolarForce>(m_nodes.size(), PolarForce{});
	for (int i = 0; i < m_nodes.size(); i++) {
		m_nodes[i]->m_id = i;
	}
	for (int i = 0; i < m_nodes.size(); i++) {
		const auto& node = m_nodes[i];
		if (node->getType() == Node::ConnectionType::kSubdivision) {
			applySmoothingForce(i, m_nodes[i]->m_parent->m_id, m_nodes[i]->m_children[0]->m_id);
		}
	}
	for (int i = 0; i < m_nodes.size(); i++) {
		Number<Inexact> epsilon = 0.001;
		if (m_nodes[i]->getType() == Node::ConnectionType::kJoin) {
			m_nodes[i]->m_position.setR(m_nodes[i]->m_position.r() + epsilon * m_forces[i].r);
		}
		if (m_nodes[i]->getType() == Node::ConnectionType::kJoin ||
		    m_nodes[i]->getType() == Node::ConnectionType::kSubdivision) {
			m_nodes[i]->m_position.setPhi(m_nodes[i]->m_position.phi() + epsilon * m_forces[i].phi);
		}
	}
}

} // namespace cartocrow::flow_map
