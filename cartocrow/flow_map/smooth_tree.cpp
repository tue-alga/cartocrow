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

SmoothTree::SmoothTree(const std::shared_ptr<SpiralTree> spiralTree)
    : m_tree(spiralTree), m_restrictingAngle(spiralTree->restrictingAngle()) {
	Number<Inexact> rMax = 0;
	for (const auto& node : m_tree->nodes()) {
		if (node->m_position.r() > rMax) {
			rMax = node->m_position.r();
		}
	}
	constructSmoothTree(spiralTree->root(), rMax / 10); // TODO make 10 configurable

	// set node IDs
	for (int i = 0; i < m_nodes.size(); i++) {
		m_nodes[i]->m_id = i;
	}
}

std::shared_ptr<Node> SmoothTree::constructSmoothTree(const std::shared_ptr<Node>& node,
                                                      Number<Inexact> maxRStep) {
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
				const PolarPoint position = spiral.evaluate(
				    spiral.parameterForR(rMin + i * (rMax - rMin) / subdivisionCount));
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

Number<Inexact> SmoothTree::computeObstacleCost() {
	Number<Inexact> cost = 0;
	for (int i = 0; i < m_nodes.size(); i++) {
		const auto& node = m_nodes[i];
		if (node->getType() == Node::ConnectionType::kSubdivision ||
		    node->getType() == Node::ConnectionType::kJoin) {
			for (int j = 0; j < m_nodes.size(); j++) {
				const auto& obstacle = m_nodes[j];
				if (obstacle->getType() == Node::ConnectionType::kLeaf) {
					cost += computeObstacleCost(i, node->m_flow, obstacle->m_position);
				}
			}
		}
	}
	return cost;
}

Number<Inexact> SmoothTree::computeObstacleCost(int i, Number<Inexact> thickness,
                                                PolarPoint obstacle) {
	PolarPoint n = m_nodes[i]->m_position;
	Number<Inexact> d = std::sqrt((n.toCartesian() - obstacle.toCartesian()).squared_length());

	if (d < thickness) {
		return thickness * (m_bufferSize / 2 + thickness) / (m_bufferSize * d) +
		       d * (m_bufferSize / 2 - thickness) / (m_bufferSize * thickness);
	} else if (d < thickness + m_bufferSize) {
		return std::pow(1 - (d - thickness) / m_bufferSize, 2);
	} else {
		return 0;
	}
}

Number<Inexact> SmoothTree::computeSmoothingCost() {
	Number<Inexact> cost = 0;
	for (int i = 0; i < m_nodes.size(); i++) {
		const auto& node = m_nodes[i];
		if (node->getType() == Node::ConnectionType::kSubdivision) {
			cost +=
			    computeSmoothingCost(i, m_nodes[i]->m_parent->m_id, m_nodes[i]->m_children[0]->m_id);
		}
	}
	return cost;
}

Number<Inexact> SmoothTree::computeSmoothingCost(int i, int iParent, int iChild) {
	PolarPoint n = m_nodes[i]->m_position;
	PolarPoint p = m_nodes[iParent]->m_position;
	PolarPoint c = m_nodes[iChild]->m_position;
	return m_smoothingFactor * std::pow(Spiral::alpha(p, n) - Spiral::alpha(n, c), 2);
}

void SmoothTree::applySmoothingGradient(int i, int iParent, int iChild) {
	PolarPoint n = m_nodes[i]->m_position;
	PolarPoint p = m_nodes[iParent]->m_position;
	PolarPoint c = m_nodes[iChild]->m_position;

	m_gradient[i].r += 2 * m_smoothingFactor * (Spiral::alpha(p, n) - Spiral::alpha(n, c)) *
	                   (Spiral::dAlphaDR2(p, n) - Spiral::dAlphaDR1(n, c));
	m_gradient[i].phi += 2 * m_smoothingFactor * (Spiral::alpha(p, n) - Spiral::alpha(n, c)) *
	                     (Spiral::dAlphaDPhi2(p, n) - Spiral::dAlphaDPhi1(n, c));

	m_gradient[iParent].r += 2 * m_smoothingFactor * (Spiral::alpha(p, n) - Spiral::alpha(n, c)) *
	                         Spiral::dAlphaDR1(p, n);
	m_gradient[iParent].phi += 2 * m_smoothingFactor * (Spiral::alpha(p, n) - Spiral::alpha(n, c)) *
	                           Spiral::dAlphaDPhi1(p, n);

	m_gradient[iChild].r += 2 * m_smoothingFactor * (Spiral::alpha(p, n) - Spiral::alpha(n, c)) *
	                        -Spiral::dAlphaDR2(n, c);
	m_gradient[iChild].phi += 2 * m_smoothingFactor * (Spiral::alpha(p, n) - Spiral::alpha(n, c)) *
	                          -Spiral::dAlphaDPhi2(n, c);
}

Number<Inexact> SmoothTree::computeAngleRestrictionCost() {
	Number<Inexact> cost = 0;
	for (int i = 0; i < m_nodes.size(); i++) {
		const auto& node = m_nodes[i];
		if (node->getType() == Node::ConnectionType::kJoin) {
			cost += computeAngleRestrictionCost(
			    i, m_nodes[i]->m_children[0]->m_id,
			    m_nodes[i]->m_children[m_nodes[i]->m_children.size() - 1]->m_id);
		}
	}
	return cost;
}

Number<Inexact> SmoothTree::computeAngleRestrictionCost(int i, int iChild1, int iChild2) {
	PolarPoint n = m_nodes[i]->m_position;
	PolarPoint c1 = m_nodes[iChild1]->m_position;
	PolarPoint c2 = m_nodes[iChild2]->m_position;
	return m_angle_restrictionFactor * (std::log(1.0 / std::cos(Spiral::alpha(n, c1))) +
	                                    std::log(1.0 / std::cos(Spiral::alpha(n, c2))));
}

void SmoothTree::applyAngleRestrictionGradient(int i, int iChild1, int iChild2) {
	PolarPoint n = m_nodes[i]->m_position;
	PolarPoint c1 = m_nodes[iChild1]->m_position;
	PolarPoint c2 = m_nodes[iChild2]->m_position;

	m_gradient[i].r +=
	    m_angle_restrictionFactor * (Spiral::dAlphaDR1(n, c1) * std::tan(Spiral::alpha(n, c1)) +
	                                 Spiral::dAlphaDR1(n, c2) * std::tan(Spiral::alpha(n, c2)));
	m_gradient[i].phi +=
	    m_angle_restrictionFactor * (Spiral::dAlphaDPhi1(n, c1) * std::tan(Spiral::alpha(n, c1)) +
	                                 Spiral::dAlphaDPhi1(n, c2) * std::tan(Spiral::alpha(n, c2)));

	m_gradient[iChild1].r +=
	    m_angle_restrictionFactor * Spiral::dAlphaDR2(n, c1) * std::tan(Spiral::alpha(n, c1));
	m_gradient[iChild1].phi +=
	    m_angle_restrictionFactor * Spiral::dAlphaDPhi2(n, c1) * std::tan(Spiral::alpha(n, c1));

	m_gradient[iChild2].r +=
	    m_angle_restrictionFactor * Spiral::dAlphaDR2(n, c2) * std::tan(Spiral::alpha(n, c2));
	m_gradient[iChild2].phi +=
	    m_angle_restrictionFactor * Spiral::dAlphaDPhi2(n, c2) * std::tan(Spiral::alpha(n, c2));
}

Number<Inexact> SmoothTree::computeBalancingCost() {
	Number<Inexact> cost = 0;
	for (int i = 0; i < m_nodes.size(); i++) {
		const auto& node = m_nodes[i];
		if (node->getType() == Node::ConnectionType::kJoin) {
			cost +=
			    computeBalancingCost(i, m_nodes[i]->m_children[0]->m_id,
			                         m_nodes[i]->m_children[m_nodes[i]->m_children.size() - 1]->m_id);
		}
	}
	return cost;
}

Number<Inexact> SmoothTree::computeBalancingCost(int i, int iChild1, int iChild2) {
	PolarPoint n = m_nodes[i]->m_position;
	PolarPoint c1 = m_nodes[iChild1]->m_position;
	PolarPoint c2 = m_nodes[iChild2]->m_position;
	return m_angle_restrictionFactor * 2 * std::pow(std::tan(m_restrictingAngle), 2) *
	       std::log(1 / std::sin(0.5 * (Spiral::alpha(n, c1) - Spiral::alpha(n, c2))));
}

void SmoothTree::applyBalancingGradient(int i, int iChild1, int iChild2) {
	PolarPoint n = m_nodes[i]->m_position;
	PolarPoint c1 = m_nodes[iChild1]->m_position;
	PolarPoint c2 = m_nodes[iChild2]->m_position;

	m_gradient[i].r += m_angle_restrictionFactor * -std::pow(std::tan(m_restrictingAngle), 2) *
	                   (1 / std::tan(0.5 * (Spiral::alpha(n, c1) - Spiral::alpha(n, c2)))) *
	                   (Spiral::dAlphaDR1(n, c1) - Spiral::dAlphaDR1(n, c2));
	m_gradient[i].phi += m_angle_restrictionFactor * -std::pow(std::tan(m_restrictingAngle), 2) *
	                     (1 / std::tan(0.5 * (Spiral::alpha(n, c1) - Spiral::alpha(n, c2)))) *
	                     (Spiral::dAlphaDPhi1(n, c1) - Spiral::dAlphaDPhi1(n, c2));

	m_gradient[iChild1].r += m_angle_restrictionFactor * -std::pow(std::tan(m_restrictingAngle), 2) *
	                         (1 / std::tan(0.5 * (Spiral::alpha(n, c1) - Spiral::alpha(n, c2)))) *
	                         Spiral::dAlphaDR2(n, c1);
	m_gradient[iChild1].phi += m_angle_restrictionFactor *
	                           -std::pow(std::tan(m_restrictingAngle), 2) *
	                           (1 / std::tan(0.5 * (Spiral::alpha(n, c1) - Spiral::alpha(n, c2)))) *
	                           Spiral::dAlphaDPhi2(n, c1);

	m_gradient[iChild2].r += m_angle_restrictionFactor * -std::pow(std::tan(m_restrictingAngle), 2) *
	                         (1 / std::tan(0.5 * (Spiral::alpha(n, c1) - Spiral::alpha(n, c2)))) *
	                         -Spiral::dAlphaDR2(n, c2);
	m_gradient[iChild2].phi += m_angle_restrictionFactor *
	                           -std::pow(std::tan(m_restrictingAngle), 2) *
	                           (1 / std::tan(0.5 * (Spiral::alpha(n, c1) - Spiral::alpha(n, c2)))) *
	                           -Spiral::dAlphaDPhi2(n, c2);
}

Number<Inexact> SmoothTree::computeStraighteningCost() {
	Number<Inexact> cost = 0;
	for (int i = 0; i < m_nodes.size(); i++) {
		const auto& node = m_nodes[i];
		if (node->getType() == Node::ConnectionType::kJoin) {
			cost += computeStraighteningCost(i, m_nodes[i]->m_parent->m_id, m_nodes[i]->m_children);
		}
	}
	return cost;
}

Number<Inexact>
SmoothTree::computeStraighteningCost(int i, int iParent,
                                     const std::vector<std::shared_ptr<Node>>& children) {
	PolarPoint n = m_nodes[i]->m_position;
	PolarPoint p = m_nodes[iParent]->m_position;
	Number<Inexact> maxFlow = 0;
	for (const auto& child : children) {
		if (child->m_flow > maxFlow) {
			maxFlow = child->m_flow;
		}
	}
	Number<Inexact> numerator = 0;
	Number<Inexact> denominator = 0;
	for (const auto& child : children) {
		if (child->m_flow >= m_relevantFlowFactor * maxFlow) {
			PolarPoint c = child->m_position;
			numerator += child->m_flow * Spiral::alpha(n, c);
			denominator += child->m_flow;
		}
	}
	return m_straighteningFactor * std::pow(Spiral::alpha(p, n) - numerator / denominator, 2);
}

void SmoothTree::applyStraighteningGradient(int i, int iParent,
                                            const std::vector<std::shared_ptr<Node>>& children) {
	PolarPoint n = m_nodes[i]->m_position;
	PolarPoint p = m_nodes[iParent]->m_position;
	Number<Inexact> maxFlow = 0;
	for (const auto& child : children) {
		if (child->m_flow > maxFlow) {
			maxFlow = child->m_flow;
		}
	}
	Number<Inexact> numerator = 0;
	Number<Inexact> numeratorDR1 = 0;
	Number<Inexact> numeratorDPhi1 = 0;
	Number<Inexact> denominator = 0;
	for (const auto& child : children) {
		if (child->m_flow >= m_relevantFlowFactor * maxFlow) {
			PolarPoint c = child->m_position;
			numerator += child->m_flow * Spiral::alpha(n, c);
			numeratorDR1 += child->m_flow * Spiral::dAlphaDR1(n, c);
			numeratorDPhi1 += child->m_flow * Spiral::dAlphaDPhi1(n, c);
			denominator += child->m_flow;
		}
	}
	m_gradient[i].r += 2 * m_straighteningFactor * (Spiral::alpha(p, n) - numerator / denominator) *
	                   (Spiral::dAlphaDR2(p, n) - numeratorDR1 / denominator);
	m_gradient[i].phi += 2 * m_straighteningFactor *
	                     (Spiral::alpha(p, n) - numerator / denominator) *
	                     (Spiral::dAlphaDPhi2(p, n) - numeratorDPhi1 / denominator);

	m_gradient[iParent].r += 2 * m_straighteningFactor *
	                         (Spiral::alpha(p, n) - numerator / denominator) *
	                         Spiral::dAlphaDR1(p, n);
	m_gradient[iParent].phi += 2 * m_straighteningFactor *
	                           (Spiral::alpha(p, n) - numerator / denominator) *
	                           Spiral::dAlphaDPhi1(p, n);

	for (const auto& child : children) {
		if (child->m_flow > maxFlow) {
			PolarPoint c = child->m_position;
			m_gradient[child->m_id].r += 2 * m_straighteningFactor *
			                             (Spiral::alpha(p, n) - numerator / denominator) *
			                             -child->m_flow * Spiral::dAlphaDR2(n, c) / denominator;
			m_gradient[child->m_id].phi += 2 * m_straighteningFactor *
			                               (Spiral::alpha(p, n) - numerator / denominator) *
			                               -child->m_flow * Spiral::dAlphaDPhi2(n, c) / denominator;
		}
	}
}

Number<Inexact> SmoothTree::computeCost() {
	return computeObstacleCost() + computeSmoothingCost() + computeAngleRestrictionCost() +
	       computeBalancingCost() + computeStraighteningCost();
}

void SmoothTree::optimize() {
	m_gradient = std::vector<PolarGradient>(m_nodes.size(), PolarGradient{});
	for (int i = 0; i < m_nodes.size(); i++) {
		const auto& node = m_nodes[i];
		if (node->getType() == Node::ConnectionType::kSubdivision) {
			applySmoothingGradient(i, m_nodes[i]->m_parent->m_id, m_nodes[i]->m_children[0]->m_id);
		} else if (node->getType() == Node::ConnectionType::kJoin) {
			applyAngleRestrictionGradient(
			    i, m_nodes[i]->m_children[0]->m_id,
			    m_nodes[i]->m_children[m_nodes[i]->m_children.size() - 1]->m_id);
			applyBalancingGradient(
			    i, m_nodes[i]->m_children[0]->m_id,
			    m_nodes[i]->m_children[m_nodes[i]->m_children.size() - 1]->m_id);
			applyStraighteningGradient(
			    i, m_nodes[i]->m_parent->m_id,
			    m_nodes[i]->m_children);
		}
	}
	for (int i = 0; i < m_nodes.size(); i++) {
		Number<Inexact> epsilon = 0.0001; // TODO
		if (m_nodes[i]->getType() == Node::ConnectionType::kJoin) {
			//m_nodes[i]->m_position.setR(m_nodes[i]->m_position.r() - epsilon * m_gradient[i].r);
		}
		if (m_nodes[i]->getType() == Node::ConnectionType::kJoin ||
		    m_nodes[i]->getType() == Node::ConnectionType::kSubdivision) {
			m_nodes[i]->m_position.setPhi(m_nodes[i]->m_position.phi() - epsilon * m_gradient[i].phi);
		}
	}
}

} // namespace cartocrow::flow_map
