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

#ifndef CARTOCROW_FLOW_MAP_SWEEP_EDGE_H
#define CARTOCROW_FLOW_MAP_SWEEP_EDGE_H

#include "../core/core.h"
#include "polar_point.h"
#include "polar_segment.h"
#include "spiral_segment.h"
#include "sweep_interval.h"

namespace cartocrow::flow_map {

/// The shape of an edge we are sweeping over: either a line segment or a spiral
/// segment. A sweep edge shape can be seen as a function that maps \f$r\f$ to
/// \f$\phi\f$.
///
/// Whether a SweepEdgeShape is a segment or a spiral is determined by its \ref
/// type(). If the shape is a spiral, it can be either a left or a right spiral.
///
/// A segment is determined by two endpoints, `start` and `end`. We interpret
/// this as a directed segment from `start` to `end`. The endpoint that is
/// furthest away from the origin is called the *far endpoint*; the other
/// endpoint is called the *near endpoint*.
///
/// A (left or right) spiral is completely determined by at a single point lying
/// on the spiral (and its restricting angle \f$\alpha\f$, which we assume is
/// identical for all sweep edges). This point is stored as `start`. In this
/// case, specifying the `end` is optional. If it is specified, then it has to
/// be further away from the origin than `start` (to make a bounded spiral
/// segment from `start` to `end`). Otherwise, we interpret this shape as
/// starting at `start` and ending at infinity.
///
/// ## Floating-point inaccuracy considerations
///
/// SweepEdgeShape and the flow_map module in general do not use CGAL's exact
/// arithmetic (\ref Exact), because the shape of a logarithmic spiral cannot be
/// expressed with rational numbers. (For example, in our algorithms we often
/// need to compute the intersection of a segment and a spiral, for which we
/// don't know any closed-form expression.) Therefore, we calculate with inexact
/// (floating-point) arithmetic and hence we have to be careful not to return
/// incorrect results due to rounding errors.
///
/// The implementation of SweepEdgeShape take some precautions to try to
/// minimize any floating-point inaccuracies involved. Firstly, the endpoints of
/// an edge shape are stored in polar coordinates, so that when the sweep circle
/// hits an endpoint, it can be set to the exact radius of that endpoint.
/// Secondly, SweepEdgeShape guarantees that if the requested radius is exactly
/// that of one of the endpoints of the shape, \ref evalForR() and \ref
/// phiForR() produce exactly the \f$\phi\f$ value of the corresponding
/// endpoint. This means that if two edges start at the same point, and the
/// sweep circle's radius is set to exactly the \f$r\f$ of that point, then \ref
/// phiForR() will return the same \f$\phi\f$ for both edges. This guarantee is
/// essential for \ref SweepCircle to work properly, as if the \f$\phi\f$ would
/// differ even a little bit, then the edges may be inserted onto the circle in
/// the incorrect order.
///
/// Unfortunately, this approach does not alleviate any and all floating-point
/// inaccuracy issues. If that events on the sweep circle happen very close
/// together, rounding errors can still cause out-of-order \f$\phi\f$ values. In
/// this case, there is essentially nothing we can do to avoid problems. Such
/// issues seem to be very rare in practice, however.
class SweepEdgeShape {
  public:
	/// Possible types of sweep edge shapes.
	enum class Type {
		/// The shape is a line segment.
		SEGMENT,
		/// The shape is a left spiral (which approaches the origin while
		/// winding around it in clockwise direction).
		LEFT_SPIRAL,
		/// The shape is a right spiral (which approaches the origin while
		/// winding around it in counter-clockwise direction).
		RIGHT_SPIRAL
	};

	/// Creates a new line segment sweep edge shape, with the given endpoints.
	SweepEdgeShape(PolarPoint start, PolarPoint end);
	/// Creates a new spiral sweep edge shape of the given type, with the given
	/// start point and angle.
	SweepEdgeShape(Type type, PolarPoint start, Number<Inexact> alpha);

	/// Returns the type of this edge shape.
	Type type() const;

	/// Returns the start point of this sweep edge shape.
	PolarPoint start() const;
	/// Returns the end point of this sweep edge shape.
	std::optional<PolarPoint> end() const;

	/// Prunes this edge shape so that the near endpoint now lies at the given
	/// point `newNear`. It is assumed that `newNear` lies on (or, due to rounding
	/// errors, at least close to) this edge shape.
	void pruneNearSide(PolarPoint newNear);
	/// Prunes this edge shape so that the far endpoint now lies at the given
	/// point `newFar`. It is assumed that `newFar` lies on (or, due to rounding
	/// errors, at least close to) this edge shape.
	void pruneFarSide(PolarPoint newFar);

	/// Returns the endpoint of this sweep edge shape closer to the origin.
	PolarPoint nearEndpoint() const;
	/// Returns the endpoint of this sweep edge shape further to the origin.
	std::optional<PolarPoint> farEndpoint() const;

	/// Returns the \f$r\f$ of the endpoint of this sweep edge shape closer to
	/// the origin.
	Number<Inexact> nearR() const;
	/// Returns the \f$r\f$ of the endpoint of this sweep edge shape further
	/// from the origin.
	std::optional<Number<Inexact>> farR() const;
	/// Returns the average \f$r\f$ of the endpoints of this sweep edge shape.
	std::optional<Number<Inexact>> averageR() const;

	/// Returns the angle \f$\phi\f$ at which this sweep edge shape intersects a
	/// circle at radius \f$r\f$.
	Number<Inexact> phiForR(Number<Inexact> r) const;
	/// Returns the intersection of this sweep edge shape with a circle at
	/// radius \f$r\f$.
	PolarPoint evalForR(Number<Inexact> r) const;

	/// Checks if at \f$\r + \varepsilon\f$ this shape is to the left of the
	/// given shape.
	bool departsOutwardsToLeftOf(Number<Inexact> r, const SweepEdgeShape& shape) const;
	/// Computes the intersection (if any) of this sweep edge with another sweep
	/// edge. Reports the smallest \f$r\f$ of the intersections larger than \c
	/// rMin. If both this edge and the other edge are a segment, then this
	/// returns \ref std::nullopt.
	/// \todo Not supporting segments is a bit weird...
	std::optional<Number<Inexact>> intersectOutwardsWith(const SweepEdgeShape& other,
	                                                     Number<Inexact> rMin) const;
	/// Checks if at \f$\r - \varepsilon\f$ this shape is to the left of the
	/// given shape.
	bool departsInwardsToLeftOf(Number<Inexact> r, const SweepEdgeShape& shape) const;
	/// Computes the intersection (if any) of this sweep edge with another sweep
	/// edge. Reports the largest \f$r\f$ of the intersections smaller than \c
	/// rMax. If both this edge and the other edge are a segment, then this
	/// returns \ref std::nullopt.
	std::optional<Number<Inexact>> intersectInwardsWith(const SweepEdgeShape& other,
	                                                    Number<Inexact> rMax) const;

	PolarSegment toPolarSegment() const;
	SpiralSegment toSpiralSegment() const;

	bool operator==(const SweepEdgeShape& s) const = default;

  private:
	/// The type of this sweep edge.
	Type m_type;
	/// The start point.
	mutable PolarPoint m_start;
	/// The end point, if this is a segment.
	mutable std::optional<PolarPoint> m_end;
	/// The angle, if this is a spiral.
	Number<Inexact> m_alpha;
};

/// An edge intersected by the sweep circle.
class SweepEdge {
  public:
	/// Creates a new sweep edge with the given shape.
	explicit SweepEdge(SweepEdgeShape shape);

	/// Returns the shape of this edge.
	SweepEdgeShape& shape();

	/// Returns the right (i.e., previous) edge pointer.
	SweepEdge* previousEdge();
	/// Returns the right (i.e., previous) interval pointer.
	SweepInterval* previousInterval();
	/// Returns the left (i.e., next) interval pointer.
	SweepInterval* nextInterval();
	/// Returns the left (i.e., next) edge pointer.
	SweepEdge* nextEdge();

	/// Returns whether this edge is currently on the sweep circle. (This is
	/// maintained by \ref SweepCircle.)
	bool isOnCircle() const;

  private:
	/// The shape of this sweep edge.
	SweepEdgeShape m_shape;
	/// The previous edge on the sweep circle (that is, the one to the right of
	/// this edge) or \c nullptr if this is the first edge.
	SweepInterval* m_previousInterval;
	/// The next interval (that is, the one to the left of this edge).
	SweepInterval m_nextInterval;
	/// Whether the edge is currently on the sweep circle.
	bool m_onCircle = false;

	friend class SweepCircle;
};

} // namespace cartocrow::flow_map

#endif //CARTOCROW_FLOW_MAP_SWEEP_EDGE_H
