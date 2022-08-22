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

#ifndef CARTOCROW_FLOW_MAP_SPIRAL_TREE_UNOBSTRUCTED_ALGORTIHM_H
#define CARTOCROW_FLOW_MAP_SPIRAL_TREE_UNOBSTRUCTED_ALGORTIHM_H

#include "spiral_tree.h"

#include "../renderer/geometry_painting.h"
#include "../renderer/painting_renderer.h"

namespace cartocrow::flow_map {

/// Implementation of the algorithm to compute a spiral tree without obstacles.
class SpiralTreeUnobstructedAlgorithm {

  public:
	/// Constructs this class to run the algorithm for the given spiral tree.
	SpiralTreeUnobstructedAlgorithm(SpiralTree& tree);

	/// Runs the algorithm.
	void run();
	/// Returns a \ref GeometryPainting that shows some debug information. This
	/// painting shows some debug information about the algorithm run. If this
	/// method is called before \ref run(), this will result in an empty
	/// painting.
	std::shared_ptr<renderer::GeometryPainting> debugPainting();

  private:
	/// The spiral tree we are computing.
	SpiralTree& m_tree;

	struct Event {
		Event(std::shared_ptr<Node> node, const PolarPoint& relative_position)
		    : m_node(node), m_relative_position(relative_position) {}

		Event(const Event& event)
		    : m_node(event.m_node), m_relative_position(event.m_relative_position) {}

		std::shared_ptr<Node> m_node;
		PolarPoint m_relative_position;
	};
	struct CompareEvents {
		bool operator()(const Event& a, const Event& b) const {
			// join nodes are conceptually farther from the root than other nodes
			if (a.m_relative_position.r() == b.m_relative_position.r()) {
				return b.m_node->m_children.size() > 1;
			}

			return a.m_relative_position.r() < b.m_relative_position.r();
		}
	};
	using EventQueue = std::priority_queue<Event, std::deque<Event>, CompareEvents>;

	using Wavefront = std::map<Number<Inexact>, Event>;

	/// Handles a root event in the spiral tree computation algorithm.
	///
	/// This finalizes the algorithm: it connects the remaining wavefront node to
	/// the root and empties the wavefront.
	void handleRootEvent(const Event& event, Wavefront& wavefront);

	/// Handles a join event in the spiral tree computation algorithm.
	///
	/// This first checks if the event is invalid (which happens if the children
	/// of the join node are not both active anymore). If the event is valid, we
	/// remove the children from the wavefront, connect them to the join node,
	/// and add the join node to the wavefront.
	///
	/// Returns the position of the newly inserted join node in the wavefront,
	/// or \c std::nullopt if the event was invalid.
	std::optional<Wavefront::iterator> handleJoinEvent(const Event& event, Wavefront& wavefront);

	/// Handles a leaf event in the spiral tree computation algorithm.
	///
	/// This checks if the new leaf node is reachable from one of its neighbors
	/// in the wavefront. (It cannot be reachable from both neighbors, because
	/// then the reachable regions from these neighbors would overlap, resulting
	/// in a join event that should have been handled before this leaf event,
	/// which removes the neighbors from the wavefront.)
	///
	/// If indeed the leaf node is reachable from a neighbor \f$ v \f$, then
	/// \f$ v \f$ becomes the child of the new node and hence gets removed from
	/// the wavefront. Else, the leaf node is simply inserted into the wavefront
	/// without children.
	///
	/// Returns the position of the newly inserted leaf node in the wavefront.
	Wavefront::iterator handleLeafEvent(Event& event, Wavefront& wavefront);

	void insertJoinEvent(const Event& first, const Event& second, EventQueue& events);

	std::shared_ptr<renderer::PaintingRenderer> m_debugPainting;
};

} // namespace cartocrow::flow_map

#endif //CARTOCROW_FLOW_MAP_SPIRAL_TREE_UNOBSTRUCTED_ALGORTIHM_H
