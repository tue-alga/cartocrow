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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 04-09-2020
*/

#ifndef CARTOCROW_CORE_POLAR_POINT_H
#define CARTOCROW_CORE_POLAR_POINT_H

#include <cmath>
#include <ostream>

#include "../core/core.h"

namespace cartocrow::flow_map {

/// A 2D point with polar coordinates.
///
/// A polar point stores a distance \f$r \geq 0\f$ to the origin \f$(0, 0)\f$
/// and a counter-clockwise angle \f$\phi \in [-\pi, \pi)\f$. It corresponds to
/// a point with Cartesian coordinates
/// \f[
/// (r \cdot \cos \phi, r \cdot \sin \phi) \text{.}
/// \f]
class PolarPoint {
  public:
	/// Constructs a polar point at the origin.
	PolarPoint();
	/// Constructs a polar point at the origin.
	[[deprecated]] PolarPoint(const CGAL::Origin& o);
	/// Constructs a polar point with given \f$r\f$ and \f$\phi\f$.
	PolarPoint(const Number<Inexact>& R, const Number<Inexact>& phi);
	/// Copy constructor.
	PolarPoint(const PolarPoint& p);
	/// Constructs a polar point that corresponds to \f$p + t\f$, where \f$p\f$
	/// is a polar point and \f$t\f$ is a vector in Cartesian coordinates.
	PolarPoint(const PolarPoint& p, const Vector<Inexact>& t);
	/// Constructs a polar point from a point in Cartesian coordinates.
	explicit PolarPoint(const Point<Inexact>& p);
	PolarPoint(const Point<Inexact>& p, const Vector<Inexact>& t);

	/// Returns the distance \f$r\f$ from the origin.
	const Number<Inexact> r() const;
	/// Returns the distance \f$r^2\f$ from the origin.
	const Number<Inexact> rSquared() const;
	/// Returns the angle \f$\phi\f$ relative to the origin.
	const Number<Inexact> phi() const;

	/// Sets the distance \f$r\f$ from the origin.
	void setR(Number<Inexact> r);
	/// Sets the distance \f$\phi\f$ relative to the origin.
	void setPhi(Number<Inexact> phi);

	/// Returns the point in Cartesian coordinates corresponding to this polar
	/// point.
	Point<Inexact> toCartesian() const;

  private:
	/// Returns a polar point corresponding to the given point in Cartesian
	/// coordinates.
	static PolarPoint toPolar(const Point<Inexact>& p);
	/// Returns a polar point that corresponds to \f$p + t\f$, where \f$p\f$
	/// is a polar point and \f$t\f$ is a vector in Cartesian coordinates.
	static PolarPoint translate(const PolarPoint& p, const Vector<Inexact>& t);

	/// The distance from the origin.
	Number<Inexact> m_r;
	/// The angle relative to the origin.
	Number<Inexact> m_phi;
};

/// Checks if two polar points are equal.
///
/// Two polar points \f$(r_1, \phi_1)\f$ and \f$(r_2, \phi_2)\f$ are equal if
/// \f$r_1 = r_2\f$ and \f$\phi_1 = \phi_2\f$, or if \f$r_1 = r_2 = 0\f$.
bool operator==(const PolarPoint& p, const PolarPoint& q);

/// Checks if two polar points are not equal.
bool operator!=(const PolarPoint& p, const PolarPoint& q);

/// Outputs a polar point to an output stream.
std::ostream& operator<<(std::ostream& os, const PolarPoint& point);

} // namespace cartocrow::flow_map

#endif //CARTOCROW_CORE_POLAR_POINT_H
