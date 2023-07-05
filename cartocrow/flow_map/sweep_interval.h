/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
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

#ifndef CARTOCROW_FLOW_MAP_SWEEP_INTERVAL_H
#define CARTOCROW_FLOW_MAP_SWEEP_INTERVAL_H

#include <map>

#include "../core/core.h"
#include "../renderer/geometry_renderer.h"
#include "node.h"
#include "polar_point.h"

namespace cartocrow::flow_map {

class SweepEdge;

/// An interval on the sweep circle. A sweep interval maintains:
///
/// * its \ref Type "type" (reachable, obstacle, or free),
///
/// * pointers to its boundary edges on the left and right sides, and
///
/// * for reachable intervals in the second sweep: two pointers to nodes:
///     * the *child*: the closest node that this interval is reachable from,
///       and
///     * the *active descendant*: the closest non-degree-2 node that this
///       interval is reachable from (this is what is called the "tag" in
///       \cite flow_maps_algorithmica).
///
/// Often the child and the active descendant are the same. However, it can
/// happen that the child is a degree-2 node, which would route around an
/// obstacle. In this case, the active descendant is further away than the
/// child. The reason for storing both the child and the active descendant is
/// the following. Assume that the reachable interval at some point gets joined
/// with another reachable interval (see \ref
/// SpiralTreeObstructedAlgorithm::JoinEvent "join events"), thereby generating
/// a new node. At this point we need to insert a tree edge from the newly added
/// node to the child. However, we also need to remove all other reachable
/// intervals with the same active descendant. (If we wouldn't do that, then
/// later in the process these reachable intervals may also experience join
/// events, and hence we would get a cycle in the tree.)
class SweepInterval {
  public:
	/// Possible types of sweep intervals.
	enum class Type {
		/// The interval is reachable from the origin (in the first sweep) or
		/// from any of the active nodes (in the second sweep).
		REACHABLE,
		/// The interval is not reachable due to being in an obstacle.
		OBSTACLE,
		/// The interval is not reachable due to being behind an obstacle (in
		/// the first sweep).
		SHADOW,
		/// The interval is not reachable due to being outside of the spiral
		/// regions induced by the active nodes (in the second sweep).
		FREE
	};

	/// Creates a new sweep interval of the given type, with no associated
	/// previous boundary.
	explicit SweepInterval(Type type);
	/// Creates a new sweep interval whose attributes are copied from the given
	/// interval, but with the specified previous and next boundaries.
	SweepInterval(const SweepInterval& other, SweepEdge* previousBoundary, SweepEdge* nextBoundary);

	/// Returns the previous boundary edge pointer.
	SweepEdge* previousBoundary();
	/// Returns the next boundary edge pointer.
	SweepEdge* nextBoundary();

	/// Sets the type of this interval.
	void setType(Type type);
	/// Returns the type of this interval.
	Type type() const;

	/// Sets the node this interval is reachable from. This is applicable only
	/// for reachable intervals in the second sweep.
	void setNode(std::shared_ptr<Node> node);
	/// Returns the node this interval is reachable from. This is applicable
	/// only for reachable intervals in the second sweep.
	std::shared_ptr<Node> node() const;
	/// Sets the active descendant of this interval. This is applicable only for
	/// reachable intervals in the second sweep.
	void setActiveDescendant(std::shared_ptr<Node> activeDescendant);
	/// Returns the active descendant of this interval. This is applicable only
	/// for reachable intervals in the second sweep.
	std::shared_ptr<Node> activeDescendant() const;

	/// Computes the point of intersection larger than \c rMin of the two sides
	/// of this interval. This returns \ref std::nullopt if the sides never
	/// intersect.
	std::optional<PolarPoint> outwardsVanishingPoint(Number<Inexact> rMin) const;
	/// Computes the point of intersection smaller than \c rMax of the two sides
	/// of this interval. This returns \ref std::nullopt if the sides never
	/// intersect.
	std::optional<PolarPoint> inwardsVanishingPoint(Number<Inexact> rMax) const;

	/// Returns a piecewise linear approximation of the shape swept by this
	/// interval within the given \f$r\f$ interval. This is meant for debugging
	/// purposes, to allow rendering the interval.
	Polygon<Inexact> sweepShape(Number<Inexact> rFrom, Number<Inexact> rTo) const;
	/// Paints a sweep shape (see \ref sweepShape()) with a color determined by
	/// the type of this interval.
	void paintSweepShape(renderer::GeometryRenderer& renderer, Number<Inexact> rFrom,
	                     Number<Inexact> rTo) const;

  private:
	/// The type of this sweep interval.
	Type m_type;
	/// The sweep edge forming the previous (that is, right) boundary of this
	/// sweep circle interval, or \c nullptr if this is the first interval.
	SweepEdge* m_previousBoundary;
	/// The sweep edge forming the next (that is, left) boundary of this sweep
	/// circle interval, or \c nullptr if this is the last interval.
	SweepEdge* m_nextBoundary;
	/// If this is a reachable interval in the second sweep, this field stores
	/// the node it is reachable from. Otherwise, it is \c nullptr.
	std::shared_ptr<Node> m_node = nullptr;
	/// If this is a reachable interval in the second sweep, this field stores
	/// the active descendant of this interval. Otherwise, it is \c nullptr.
	std::shared_ptr<Node> m_activeDescendant = nullptr;

	friend class SweepCircle;
};

} // namespace cartocrow::flow_map

#endif //CARTOCROW_FLOW_MAP_SWEEP_INTERVAL_H
