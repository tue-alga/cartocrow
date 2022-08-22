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

#ifndef CARTOCROW_CORE_POLAR_LINE_H
#define CARTOCROW_CORE_POLAR_LINE_H

#include <ostream>

#include "../core/core.h"
#include "polar_point.h"

namespace cartocrow::flow_map {

/// A straight line represented in polar coordinates.
///
/// A polar line \f$l\f$ can be represented by its *foot* \f$f_l\f$, which is
/// the point on \f$l\f$ closest to the origin.
///
/// \image html polar-line-foot.svg
///
/// Points on the line can be characterized by a parameter \f$t\f$, the signed
/// distance along the line, measured from the foot. Points to the left of the
/// foot (relative to the origin-foot segment) have \f$t > 0\f$, points to the
/// right have \f$t < 0\f$, and the foot itself has \f$t = 0\f$. The method
/// \ref pointAlongLine() returns the point with a given signed distance. To
/// avoid confusion, the documentation for this class uses \f$t\f$ to denote
/// the signed distance along the line of a point on the line, and \f$r\f$ to
/// denote the distance of a point from the origin.
///
/// \todo It is not specified what happens if the foot coincides with the
/// origin. This should probably be avoided.
class PolarLine {

  public:
	/// Constructs a polar line with the given foot.
	explicit PolarLine(const PolarPoint& foot);
	/// Constructs the polar line through the two given points.
	PolarLine(const PolarPoint& p1, const PolarPoint& p2);

	/// Returns the foot of this polar line.
	const PolarPoint& foot() const;
	/// Returns the foot of this polar line.
	PolarPoint& foot();

	/// Checks whether this line contains any polar point \f$(r, \phi)\f$ with
	/// the given \f$r\f$.
	bool containsR(const Number<Inexact>& R) const;
	/// Checks whether this line contains any polar point \f$(r, \phi)\f$ with
	/// the given \f$\phi\f$.
	bool containsPhi(const Number<Inexact>& phi) const;

	/// Returns the point \f$p\f$ on this polar line at signed distance \f$t\f$
	/// from the foot.
	/// \image html polar-line-point-along-line.svg
	PolarPoint pointAlongLine(const Number<Inexact>& t) const;

	/// Computes the signed distance \f$t\f$ along the line to the point on
	/// the line at the given \f$\phi\f$.
	/// \image html polar-line-distance-along-line-for-phi.svg
	/// Throws if such point(s) do not exist.
	Number<Inexact> distanceAlongLineForPhi(const Number<Inexact>& phi) const;

	template <class OutputIterator> int collectT(const Number<Inexact>& R, OutputIterator t) const;

	/// Computes the distance \f$r\f$ to the point(s) on the line at the
	/// given \f$\phi\f$.
	/// \image html polar-line-distance-for-phi.svg
	/// Throws if such point(s) do not exist.
	Number<Inexact> distanceForPhi(const Number<Inexact>& phi) const;

	template <class OutputIterator>
	int collectPhi(const Number<Inexact>& R, OutputIterator phi) const;

	/// Computes the angle \f$\alpha\f$ at a point \f$p\f$ on this polar line
	/// at distance \f$r\f$ from the origin, between the line segment from the
	/// origin to \f$p\f$ and this polar line.
	/// \image html polar-line-tangent-angle.svg
	std::optional<Number<Inexact>> tangentAngle(const Number<Inexact>& r) const;

  protected:
	// Returns the signed distance from point 1 to point 2.
	Number<Inexact> setFoot(const PolarPoint& point_1, const PolarPoint& point_2);

  private:
	/// The foot of this polar line.
	PolarPoint m_foot;
};

std::ostream& operator<<(std::ostream& os, const PolarLine& line);

} // namespace cartocrow::flow_map

#include "polar_line.inc"

#endif //CARTOCROW_CORE_POLAR_LINE_H
