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

#ifndef CARTOCROW_FLOW_MAP_SWEEP_CIRCLE_H
#define CARTOCROW_FLOW_MAP_SWEEP_CIRCLE_H

#include <map>
#include <ostream>

#include "../core/core.h"
#include "cartocrow/flow_map/polar_point.h"

namespace cartocrow::flow_map {

class SweepEdge;

/// An interval on the sweep circle. A sweep interval maintains its type
/// (reachable, obstacle, or free) and pointers to its boundary edges on the
/// left and right sides.
class SweepInterval {
  public:
	/// Possible types of sweep intervals.
	enum class Type {
		/// The interval is reachable.
		REACHABLE,
		/// The interval is not reachable due to being in an obstacle.
		OBSTACLE,
		/// The interval is not reachable due to being behind an obstacle.
		SHADOW
	};

	/// Creates a new sweep interval of the given type, with unset boundaries.
	SweepInterval(Type type);

	/// Sets the next boundary edge pointer.
	void setNextBoundary(SweepEdge* nextBoundary);
	/// Returns the next boundary edge pointer.
	SweepEdge* nextBoundary() const;
	/// Sets the previous boundary edge pointer.
	void setPreviousBoundary(SweepEdge* previousBoundary);
	/// Returns the previous boundary edge pointer.
	SweepEdge* previousBoundary() const;

	/// Sets the type of this interval.
	void setType(Type type);
	/// Returns the type of this interval.
	Type type() const;

  private:
	/// The type of this sweep interval.
	Type m_type;
	/// The sweep edge forming the next (that is, left) boundary of this sweep
	/// circle interval.
	SweepEdge* m_nextBoundary;
	/// The sweep edge forming the previous (that is, right) boundary of this
	/// sweep circle interval.
	SweepEdge* m_previousBoundary;
};

/// The shape of a sweep edge: either a line segment or a spiral segment.
class SweepEdgeShape {
  public:
	/// Possible types of sweep edges.
	enum class Type {
		/// The edge is a line segment.
		SEGMENT,
		/// The edge is a left spiral.
		LEFT_SPIRAL,
		/// The edge is a right spiral.
		RIGHT_SPIRAL
	};

	/// Creates a new sweep edge of the given type, with the given endpoints.
	SweepEdgeShape(Type type, PolarPoint p1, PolarPoint p2);

	/// Returns the \f$r\f$ of the endpoint of this sweep edge closer to the
	/// origin.
	Number<Inexact> nearR() const;
	/// Returns the \f$r\f$ of the endpoint of this sweep edge further from the
	/// origin.
	Number<Inexact> farR() const;

	/// Returns the angle \f$\phi\f$ at which this sweep edge intersects a
	/// circle at radius \f$r\f$.
	Number<Inexact> phiForR(Number<Inexact> r) const;

	bool operator==(const SweepEdgeShape& s) const = default;

  private:
	/// The type of this sweep edge.
	Type m_type;
	/// The endpoint closer to the origin.
	PolarPoint m_p1;
	/// The endpoint further from the origin.
	PolarPoint m_p2;
};

/// An edge intersected by the sweep circle.
class SweepEdge {
  public:
	/// Creates a new sweep edge with the given shape.
	SweepEdge(SweepEdgeShape shape);

	/// Returns the shape of this edge.
	const SweepEdgeShape& shape() const;

	/// Returns the next interval pointer.
	SweepInterval* nextInterval() const;
	/// Returns the previous interval pointer.
	SweepInterval* previousInterval() const;

  private:
	/// The shape of this sweep edge.
	SweepEdgeShape m_shape;
	/// The next interval (that is, the one to the left of this edge).
	std::shared_ptr<SweepInterval> m_nextInterval;
	/// The previous interval (that is, the one to the right of this edge).
	std::shared_ptr<SweepInterval> m_previousInterval;

	friend class SweepCircle;
};

/// Representation of the sweep circle used in the \ref
/// SpiralTreeObstructedAlgorithm.
///
/// The sweep circle stores an ordered set of the edges it intersects, and the
/// intervals between them.
class SweepCircle {
  public:
	/// Creates a sweep circle of radius 0, consisting of a single reachable
	/// interval.
	SweepCircle();

	/// Returns the current radius of the sweep circle.
	Number<Inexact> r();

	/// Sets the radius to the given value. This does not update anything
	/// structurally; in other words, it assumes that the circle does not pass
	/// over vertices or intersections. If it does, the sweep circle may become
	/// invalid (see \ref isValid()).
	void grow(Number<Inexact> r);

	/// Checks if this sweep circle is still valid, that is, if the edges and
	/// intervals in this sweep circle are still ordered in counter-clockwise
	/// order around the origin.
	bool isValid() const;

	/// Prints a summary of the edges and intervals on the sweep circle. This is
	/// meant for debug purposes.
	void print() const;

	/// Returns the number of intervals on the sweep circle.
	std::size_t intervalCount() const;

	/// Returns the interval on the circle at the given angle \f$\phi\f$. If
	/// \f$\phi\f$ exactly lies on an edge (see \ref edgeAt()), the right
	/// interval of that edge is returned.
	/// \note The pointer returned by this function is invalidated as soon as
	/// any structural changes to the sweep circle occur.
	SweepInterval* intervalAt(Number<Inexact> phi);

	/// Returns the edge at the given angle \f$\phi\f$. If there is no edge at
	/// this \f$\phi\f$, returns \c nullptr.
	/// \note The pointer returned by this function is invalidated as soon as
	/// any structural changes to the sweep circle occur.
	SweepEdge* edgeAt(Number<Inexact> phi);

	/// The elements (intervals and edges) resulting from a split operation, in
	/// order in increasing angle over the circle.
	struct SplitResult {
		SweepInterval* rightInterval;
		SweepEdge* rightEdge;
		SweepInterval* middleInterval;
		SweepEdge* leftEdge;
		SweepInterval* leftInterval;
	};

	/// Splits the given edge \f$e\f$ into two, with a new interval in between.
	/// Assumes that the far endpoint of \f$e\f$ is currently on this sweep
	/// circle, and the newly inserted edges have their near endpoints at the
	/// same point on this sweep circle.
	SplitResult splitFromEdge(SweepEdge* e, SweepEdgeShape newRightEdgeShape,
	                          SweepEdgeShape newLeftEdgeShape);
	/// Splits the given interval \f$i\f$ into two, with a new interval in
	/// between. Assumes that the newly inserted edges have their near endpoints
	/// at the same point on this sweep circle.
	SplitResult splitFromInterval(SweepInterval* i, SweepEdgeShape newRightEdgeShape,
	                              SweepEdgeShape newLeftEdgeShape);

	/// The elements (intervals and edges) resulting from a switch operation, in
	/// order in increasing angle over the circle.
	struct SwitchResult {
		SweepInterval* rightInterval;
		SweepEdge* newEdge;
		SweepInterval* leftInterval;
	};

	/// Replaces one edge \f$e\f$ by another. Assumes that the far endpoint of
	/// \f$e\f$ is currently on this sweep circle and coincides with the near
	/// endpoint of the new edge.
	SwitchResult switchEdge(SweepEdge* e, SweepEdgeShape newEdgeShape);

	/// The elements (intervals and edges) resulting from a merge operation, in
	/// order in increasing angle over the circle.
	struct MergeResult {
		SweepInterval* mergedInterval;
	};

	/// Removes two edges. Assumes that the far endpoints of both edges
	/// coincides and lies currently on this sweep circle.
	MergeResult mergeToInterval(SweepEdge* rightEdge, SweepEdge* leftEdge);

  private:
	struct SweepEdgeShapeComparator {
		SweepEdgeShapeComparator(SweepCircle* owner) : m_owner(owner) {}
		SweepCircle* m_owner;
		bool operator()(SweepEdgeShape e1, SweepEdgeShape e2) const {
			return e1.phiForR(m_owner->m_r) < e2.phiForR(m_owner->m_r);
		}
		bool operator()(Number<Inexact> phi1, SweepEdgeShape e2) const {
			return phi1 < e2.phiForR(m_owner->m_r);
		}
		bool operator()(SweepEdgeShape e1, Number<Inexact> phi2) const {
			return e1.phiForR(m_owner->m_r) < phi2;
		}
		struct is_transparent {};
	};
	/// A set containing the sweep edges separating the intervals.
	std::multimap<SweepEdgeShape, SweepEdge, SweepEdgeShapeComparator> m_edges{
	    SweepEdgeShapeComparator(this)};
	/// The first interval on the circle.
	std::shared_ptr<SweepInterval> m_firstInterval;
	/// The last interval on the circle.
	std::shared_ptr<SweepInterval> m_lastInterval;
	/// Current radius of the circle.
	Number<Inexact> m_r;
};

} // namespace cartocrow::flow_map

#endif //CARTOCROW_FLOW_MAP_SWEEP_CIRCLE_H
