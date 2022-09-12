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
#include "sweep_interval.h"

namespace cartocrow::flow_map {

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

	/// Returns the angle \f$\phi\f$ at which this sweep edge shape intersects a
	/// circle at radius \f$r\f$.
	Number<Inexact> phiForR(Number<Inexact> r) const;
	/// Returns the intersection of this sweep edge shape with a circle at
	/// radius \f$r\f$.
	PolarPoint evalForR(Number<Inexact> r) const;

	/// Checks if at \f$\epsilon\f$ away from the near endpoint of this shape
	/// and the given shape, this shape is to the left of the given shape.
	/// Assumes that \c shape has the same \ref nearEndpoint() as this shape.
	bool departsToLeftOf(const SweepEdgeShape& shape) const;

	bool operator==(const SweepEdgeShape& s) const = default;

  private:
	/// The type of this sweep edge.
	Type m_type;
	/// The start point.
	PolarPoint m_start;
	/// The end point, if this is a segment.
	std::optional<PolarPoint> m_end;
	/// The angle, if this is a spiral.
	Number<Inexact> m_alpha;
};

/// An edge intersected by the sweep circle.
class SweepEdge {
  public:
	/// Creates a new sweep edge with the given shape.
	explicit SweepEdge(SweepEdgeShape shape);

	/// Returns the shape of this edge.
	const SweepEdgeShape& shape() const;

	/// Returns the right (i.e., previous) edge pointer.
	SweepEdge* previousEdge();
	/// Returns the right (i.e., previous) interval pointer.
	SweepInterval* previousInterval();
	/// Returns the left (i.e., next) interval pointer.
	SweepInterval* nextInterval();
	/// Returns the left (i.e., next) edge pointer.
	SweepEdge* nextEdge();

  private:
	/// The shape of this sweep edge.
	SweepEdgeShape m_shape;
	/// The previous edge on the sweep circle (that is, the one to the right of
	/// this edge) or \c nullptr if this is the first edge.
	SweepInterval* m_previousInterval;
	/// The next interval (that is, the one to the left of this edge).
	SweepInterval m_nextInterval;

	friend class SweepCircle;
};

} // namespace cartocrow::flow_map

#endif //CARTOCROW_FLOW_MAP_SWEEP_EDGE_H
