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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 19-02-2021
*/

#ifndef CARTOCROW_CORE_POLAR_SEGMENT_H
#define CARTOCROW_CORE_POLAR_SEGMENT_H

#include <ostream>

#include "../core/core.h"
#include "polar_line.h"
#include "polar_point.h"

namespace cartocrow::flow_map {

/// A directed line segment \f$\overline{AB}\f$ represented in polar
/// coordinates.
///
/// A PolarSegment is represented as part of a \ref PolarLine, called its
/// *supporting line*. The segment hence forms a directed interval \f$[t_1,
/// t_2]\f$ of (signed) distances along the line (see the \ref PolarLine
/// "class documentation of PolarLine" for context).
/// \image html polar-segment.svg
///
/// To store this interval, we store its *start* \f$t_1\f$ and its *length*
/// \f$t_2 - t_1\f$. Note that the length may be positive, in which case the
/// segment is a *left line*, or negative, in which case the segment is a *right
/// line*. See \ref isLeftLine() and \ref isRightLine().
///
/// Points on a PolarSegment are parameterized such that \f$[0, 1]\f$ covers
/// the entire segment. The method \ref pointAlongSegment() returns the point
/// with a given parameter.
class PolarSegment : private PolarLine {
  public:
	/// Constructs a segment from \f$p_1\f$ to \f$p_2\f$.
	PolarSegment(const PolarPoint& p1, const PolarPoint& p2);

	/// Computes the smallest distance from any point on the line segment to the
	/// origin.
	Number<Inexact> rMin() const;
	/// Computes the largest distance from any point on the line segment to the
	/// origin.
	Number<Inexact> rMax() const;

	/// Checks if this segment is a *left line*, that is, if \f$OAB\f$ is a
	/// right-turning angle.
	bool isLeftLine() const;
	/// Checks if this segment is a *right line*, that is, if \f$OAB\f$ is a
	/// left-turning angle.
	bool isRightLine() const;
	/// Checks if \f$OAB\f$ are collinear.
	bool isCollinear() const;

	/// Checks whether this segment contains the foot of its supporting line.
	bool containsFoot() const;
	/// Checks whether this segment contains any polar point \f$(r, \phi)\f$
	/// with the given \f$r\f$.
	bool containsR(const Number<Inexact>& R) const;
	/// Checks whether this segment contains any polar point \f$(r, \phi)\f$
	/// with the given \f$\phi\f$.
	bool containsPhi(const Number<Inexact>& phi) const;

	/// Returns the point \f$p\f$ parameterized by \f$t\f$.
	PolarPoint pointAlongSegment(const Number<Inexact>& t) const;

	/// Computes the parameter \f$t\f$ of the point on this segment at the
	/// given \f$\phi\f$. Throws if such point(s) do not exist.
	Number<Inexact> parameterForPhi(const Number<Inexact>& phi) const;

	template <class OutputIterator> int collectT(const Number<Inexact>& R, OutputIterator t) const;

	template <class OutputIterator>
	int collectPhi(const Number<Inexact>& R, OutputIterator phi) const;

	/// Computes the point on this line segment closest to the origin. This may
	/// be the foot of the supporting line, or one of the endpoints.
	PolarPoint closestToOrigin() const;

	/// Returns the supporting line of this segment.
	const PolarLine& supportingLine() const;

  private:
	/// Converts a parameter \f$t\f$ to a signed distance to the foot of the
	/// supporting line.
	Number<Inexact> toDistance(const Number<Inexact>& t) const;
	/// Converts a signed distance to the root of the supporting line to a
	/// parameter \f$t\f$.
	Number<Inexact> toParameter(const Number<Inexact>& distance) const;

	/// The start of the interval, as the signed distance to the foot of the
	/// supporting line.
	Number<Inexact> m_start;
	/// The length of the interval.
	Number<Inexact> m_length;
};

std::ostream& operator<<(std::ostream& os, const PolarSegment& line);

} // namespace cartocrow::flow_map

#include "polar_segment.hpp"

#endif //CARTOCROW_CORE_POLAR_SEGMENT_H
