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

#include "cartocrow/core/cgal_types.h"
#include "cartocrow/core/core_types.h"

namespace cartocrow {

/// A 2D point with polar coordinates.
/**
 * A polar point stores a distance \f$r \geq 0\f$ to the origin \f$(0, 0)\f$
 * and a counter-clockwise angle \f$\phi \in [-\pi, \pi)\f$. It corresponds to
 * a point with Cartesian coordinates
 * \f[
 * (r \cdot \cos \phi, r \cdot \sin \phi) \text{.}
 * \f]
 */
class PolarPoint {
  public:
	/// Constructs a polar point at the origin.
	PolarPoint();

	/// Constructs a polar point at the origin.
	[[deprecated]] PolarPoint(const CGAL::Origin& o);

	/// Constructs a polar point with given \f$r\f$ and \f$\phi\f$.
	PolarPoint(const Number& R, const Number& phi);

	/// Copy constructor.
	PolarPoint(const PolarPoint& p);

	PolarPoint(const PolarPoint& p, const Vector& t);

	/// Constructs a polar point from a point in Cartesian coordinates.
	explicit PolarPoint(const Point& p);

	PolarPoint(const Point& p, const Vector& t);

	/// Returns the distance from the origin.
	const Number& R() const;
	/// Returns the angle relative to the origin.
	const Number& phi() const;

	/// Returns the point in Cartesian coordinates corresponding to this polar
	/// point.
	Point to_cartesian() const;

  private:
	static PolarPoint to_polar(const Point& p);
	static PolarPoint translate_pole(const PolarPoint& p, const Vector& t);

	/// The distance from the origin.
	Number m_r;
	/// The angle relative to the origin.
	Number m_phi;
};

/// Checks if two polar points are equal.
/**
 * Two polar points \f$(r_1, \phi_1)\f$ and \f$(r_2, \phi_2)\f$ are equal if
 * \f$r_1 = r_2\f$ and \f$\phi_1 = \phi_2\f$, or if \f$r_1 = r_2 = 0\f$.
 */
bool operator==(const PolarPoint& p, const PolarPoint& q);

/// Checks if two polar points are not equal.
bool operator!=(const PolarPoint& p, const PolarPoint& q);

/// Outputs a polar point to an output stream.
std::ostream& operator<<(std::ostream& os, const PolarPoint& point);

} // namespace cartocrow

#endif //CARTOCROW_CORE_POLAR_POINT_H
