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

#include "spiral_tree_unobstructed_algorithm.h"

#include <optional>
#include <ostream>

#include <glog/logging.h>

#include "../core/core.h"
#include "../necklace_map/circular_range.h"
#include "circulator.h"
#include "intersections.h"
#include "polar_segment.h"
#include "spiral_segment.h"

namespace cartocrow::flow_map {

SpiralTreeUnobstructedAlgorithm::SpiralTreeUnobstructedAlgorithm(SpiralTree& tree)
    : m_tree(tree), m_debugPainting(std::make_shared<renderer::PaintingRenderer>()) {}

void SpiralTreeUnobstructedAlgorithm::run() {

	// we maintain a wavefront as a BST of events, with their angle around the
	// root as the key
	Wavefront wavefront;

	// insert all terminals into the event queue
	EventQueue events;
	for (const std::shared_ptr<Node>& node : m_tree.nodes()) {
		events.push(Event(node, node->m_position));
	}

	// main loop, handle all events
	while (!events.empty()) {
		Event event = events.top();
		events.pop();

		// if we have reached the root, handle that and stop
		if (event.m_relative_position.r() == 0) {
			handleRootEvent(event, wavefront);
			break;
		}

		// handle join and leaf events
		std::optional<Wavefront::iterator> new_node;
		if (event.m_node->m_children.size() > 1) {
			new_node = handleJoinEvent(event, wavefront);
		} else {
			new_node = handleLeafEvent(event, wavefront);
		}

		// insert join events involving the node that was newly added to the
		// wavefront, with its two neighbors in the wavefront
		if (new_node && wavefront.size() >= 2) {
			Circulator<Wavefront> cw_iter = --make_circulator(*new_node, wavefront);
			insertJoinEvent(event, cw_iter->second, events);
			Circulator<Wavefront> ccw_iter = ++make_circulator(*new_node, wavefront);
			insertJoinEvent(ccw_iter->second, event, events);
		}
	}
}

void SpiralTreeUnobstructedAlgorithm::handleRootEvent(const Event& event, Wavefront& wavefront) {
	std::shared_ptr<Node> root = event.m_node;

	// if we reached the root, then the wavefront should have only one
	// node left
	CHECK_EQ(wavefront.size(), 1);
	std::shared_ptr<Node> node = wavefront.begin()->second.m_node;
	CHECK_NOTNULL(node);

	// connect the remaining node to the root
	root->m_children.push_back(node);
	node->m_parent = root;

	//std::cout << "root node " << root->m_place->m_name << ": connected to " << node->m_place->m_name
	//          << std::endl;

	wavefront.clear();
}

std::optional<SpiralTreeUnobstructedAlgorithm::Wavefront::iterator>
SpiralTreeUnobstructedAlgorithm::handleJoinEvent(const Event& event, Wavefront& wavefront) {
	CHECK_EQ(event.m_node->m_children.size(), 2);

	// if a child is not active anymore, the event is invalid
	if (event.m_node->m_children[0]->m_parent != nullptr ||
	    event.m_node->m_children[1]->m_parent != nullptr) {
		return std::nullopt;
	}

	m_debugPainting->setStroke(Color{0, 120, 240}, 1);
	m_debugPainting->draw(Circle<Inexact>(m_tree.rootPosition(), event.m_node->m_position.rSquared()));
	m_debugPainting->drawText(
	    m_tree.rootPosition() + (event.m_node->m_position.toCartesian() - CGAL::ORIGIN), "join");

	// add the join node to the wavefront and the collection of nodes
	const Number<Inexact> angle = event.m_relative_position.phi();
	Wavefront::iterator node_iter = wavefront.emplace(angle, event).first;
	m_tree.m_nodes.push_back(event.m_node);

	//std::cout << "join node " << event.m_node->m_place->m_name << ": connected to "
	//<< event.m_node->m_children[0]->m_place->m_name << " and "
	//<< event.m_node->m_children[1]->m_place->m_name << std::endl;

	// connect the children to the join node
	event.m_node->m_children[0]->m_parent = event.m_node;
	event.m_node->m_children[1]->m_parent = event.m_node;

	// remove the children from the wavefront
	CHECK_GE(wavefront.size(), 3);
	wavefront.erase(--make_circulator(node_iter, wavefront));
	wavefront.erase(++make_circulator(node_iter, wavefront));

	return node_iter;
}

SpiralTreeUnobstructedAlgorithm::Wavefront::iterator
SpiralTreeUnobstructedAlgorithm::handleLeafEvent(Event& event, Wavefront& wavefront) {

	m_debugPainting->setStroke(Color{120, 0, 240}, 1);
	m_debugPainting->draw(Circle<Inexact>(m_tree.rootPosition(), event.m_node->m_position.rSquared()));
	m_debugPainting->drawText(
	    m_tree.rootPosition() + (event.m_node->m_position.toCartesian() - CGAL::ORIGIN), "leaf");

	const Number<Inexact> angle = event.m_relative_position.phi();

	if (!wavefront.empty()) {

		// check the neighbors of the new leaf in the wavefront for reachability
		Circulator<Wavefront> node_circ = make_circulator(wavefront.lower_bound(angle), wavefront);
		if (m_tree.isReachable(event.m_relative_position, node_circ->second.m_relative_position) ||
		    m_tree.isReachable(event.m_relative_position, (--node_circ)->second.m_relative_position)) {
			// due to || being short-circuiting, node_circ now points to the
			// reachable one
			Event reachable_neighbor = node_circ->second;

			// check whether the nodes coincide
			if (event.m_relative_position == reachable_neighbor.m_relative_position) {
				// replace the event node by the join node
				reachable_neighbor.m_node->m_place = event.m_node->m_place;
				event.m_node = reachable_neighbor.m_node;
			} else {
				// connect the event node to the neighbor
				event.m_node->m_children.push_back(reachable_neighbor.m_node);
				reachable_neighbor.m_node->m_parent = event.m_node;
			}

			// remove the neighbor from the wavefront
			wavefront.erase(node_circ);
		}
	}

	//std::cout << "leaf node " << event.m_node->m_place->m_name << ": added" << std::endl;

	return wavefront.emplace(angle, event).first;
}

void SpiralTreeUnobstructedAlgorithm::insertJoinEvent(const Event& first, const Event& second,
                                                      EventQueue& events) {
	const Spiral spiral_left(first.m_relative_position, -m_tree.restrictingAngle());
	const Spiral spiral_right(second.m_relative_position, m_tree.restrictingAngle());

	std::vector<PolarPoint> intersections;
	intersect(spiral_left, spiral_right, intersections);
	CHECK(intersections.size() > 0);

	// the intersections should be the two closest to the anchor of the first spiral
	const PolarPoint& intersection = intersections[0];
	CHECK_LE(intersection.r(), first.m_relative_position.r());
	CHECK_LE(intersection.r(), second.m_relative_position.r());

	std::shared_ptr<Node> join = std::make_shared<Node>(intersection);
	join->m_children = {first.m_node, second.m_node};
	//const std::string name =
	//    "[" + first.m_node->m_place->m_name + "+" + second.m_node->m_place->m_name + "]";
	// create place -- removed

	events.push(Event(join, intersection));
}

std::shared_ptr<renderer::GeometryPainting> SpiralTreeUnobstructedAlgorithm::debugPainting() {
	return m_debugPainting;
}

} // namespace cartocrow::flow_map
