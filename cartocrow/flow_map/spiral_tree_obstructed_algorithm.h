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

#ifndef CARTOCROW_FLOW_MAP_SPIRAL_TREE_OBSTRUCTED_ALGORTIHM_H
#define CARTOCROW_FLOW_MAP_SPIRAL_TREE_OBSTRUCTED_ALGORTIHM_H

#include <variant>

#include "../renderer/geometry_painting.h"
#include "../renderer/painting_renderer.h"
#include "reachable_region_algorithm.h"
#include "spiral_tree.h"
#include "sweep_circle.h"

namespace cartocrow::flow_map {

/// Implementation of the algorithm to compute a spiral tree with obstacles. As
/// the input this algorithm gets the vertices of the unreachable regions (see
/// \ref ReachableRegionAlgorithm).
///
/// This is a sweep-circle algorithm which works by sweeping a circle inwards
/// from infinity towards the origin.
class SpiralTreeObstructedAlgorithm {

  public:
	/// Constructs this class to run the algorithm for the given spiral tree.
	SpiralTreeObstructedAlgorithm(
	    std::shared_ptr<SpiralTree> tree,
	    std::vector<ReachableRegionAlgorithm::UnreachableRegionVertex> vertices);

	/// Runs the algorithm.
	void run();

	/// Returns a \ref GeometryPainting that shows some debug information. This
	/// painting shows some debug information about the algorithm run. If this
	/// method is called before \ref run(), this will result in an empty
	/// painting.
	std::shared_ptr<renderer::GeometryPainting> debugPainting();

  private:
	/// The spiral tree we are computing.
	std::shared_ptr<SpiralTree> m_tree;

	/// The list of vertices of the unreachable region.
	std::vector<ReachableRegionAlgorithm::UnreachableRegionVertex> m_vertices;

	class Event;
	class CompareEvents;

	using EventQueue =
	    std::priority_queue<std::shared_ptr<Event>, std::vector<std::shared_ptr<Event>>, CompareEvents>;

	/// An event in the sweep circle algorithm.
	class Event {
	  public:
		enum class Type {
			NODE,
			VERTEX,
			JOIN,
			SWITCH,
		};

		Event(PolarPoint position, Type type, SpiralTreeObstructedAlgorithm* alg);
		Number<Inexact> r() const;
		Number<Inexact> phi() const;
		Type type() const;
		/// Handles this event.
		virtual void handle() = 0;
		/// Checks if this event is still valid.
		virtual bool isValid() const;

		/// Inserts join events for all edges starting at the event position.
		void insertJoinEvents();
		/// Inserts a join event for the interval vanishing with the given edge
		/// as the previous boundary. If the provided interval never vanishes,
		/// or the interval consists of two segment edges, no join event is
		/// inserted.
		void insertJoinEventFor(SweepCircle::EdgeMap::iterator previousEdge);

	  protected:
		PolarPoint m_position;
		Type m_type;
		SpiralTreeObstructedAlgorithm* m_alg;
	};

	class NodeEvent : public Event {
	  public:
		NodeEvent(PolarPoint position, SpiralTreeObstructedAlgorithm* alg);
		void handle() override;
	};

	/// An event that happens when the sweep circle hits an obstacle vertex. A
	/// vertex event is characterized by the two obstacle edges \f$e_1\f$ and
	/// \f$e_2\f$ incident to the hit vertex. We assume that the edges around
	/// the obstacle are ordered counter-clockwise, that is, traversing the
	/// obstacle boundary in counter-clockwise order, we traverse \f$e_2\f$
	/// right after \f$e_1\f$.
	///
	/// Vertex events are classified as one of four types, each of which is
	/// handled separately:
	///
	/// * A _near_ vertex event: both \f$e_1\f$ and \f$e_2\f$ lay outside the
	/// sweep circle.
	///
	/// * A _far_ vertex event: both \f$e_1\f$ and \f$e_2\f$ lay inside the sweep
	/// circle.
	///
	/// * A _left_ vertex event: \f$e_1\f$ lies outside the sweep circle, while
	/// \f$e_2\f$ lies inside it.
	///
	/// * A _right_ vertex event: \f$e_1\f$ lies inside the sweep circle, while
	/// \f$e_2\f$ lies outside it.
	class VertexEvent : public Event {
	  public:
		/// Creates a new vertex event at the given position, with the given
		/// incident obstacle edges \f$e_1\f$ and \f$e_2\f$ (in
		/// counter-clockwise order).
		VertexEvent(PolarPoint position, std::shared_ptr<SweepEdge> e1,
		            std::shared_ptr<SweepEdge> e2, SpiralTreeObstructedAlgorithm* alg);

		/// Possible vertex event types.
		enum class Side {
			LEFT,
			RIGHT,
			NEAR,
			FAR,
		};

		void handle() override;

	  private:
		Side determineSide();

		/// Handles a left vertex event.
		void handleLeft();
		/// Handles a right vertex event.
		void handleRight();
		/// Handles a near vertex event.
		void handleNear();
		/// Handles a far vertex event.
		void handleFar();

		/// The first edge (in counter-clockwise order around the obstacle,
		/// coming before \ref m_e2).
		std::shared_ptr<SweepEdge> m_e1;
		/// The second edge (in counter-clockwise order around the obstacle,
		/// coming after \ref m_e1).
		std::shared_ptr<SweepEdge> m_e2;
		/// The type of event, indicating on which side of the obstacle it
		/// occurs.
		Side m_side;
	};

	/// The sweep circle hits a point where a shadow interval vanishes. This
	/// excludes vanishing obstacle intervals, as these are handled by a vertex
	/// event instead.
	class JoinEvent : public Event {
	  public:
		JoinEvent(PolarPoint position, std::weak_ptr<SweepEdge> rightEdge,
		          std::weak_ptr<SweepEdge> leftEdge, SpiralTreeObstructedAlgorithm* alg);
		void handle() override;
		virtual bool isValid() const override;

	  private:
		/// The right edge involved in this join event.
		std::weak_ptr<SweepEdge> m_rightEdge;
		/// The left edge involved in this join event.
		std::weak_ptr<SweepEdge> m_leftEdge;
	};

	/// Comparator for events that sorts them in descending order of distance to
	/// the origin (compare \ref ReachableRegionAlgorithm::CompareEvents).
	struct CompareEvents {
		bool operator()(std::shared_ptr<Event>& a, std::shared_ptr<Event>& b) const {
			return a->r() < b->r();
		}
	};

	SweepCircle m_circle;
	EventQueue m_queue;

	std::shared_ptr<renderer::PaintingRenderer> m_debugPainting;
};

} // namespace cartocrow::flow_map

#endif //CARTOCROW_FLOW_MAP_SPIRAL_TREE_OBSTRUCTED_ALGORTIHM_H
