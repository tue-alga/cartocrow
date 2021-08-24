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

#include "polar_point.h"


namespace cartocrow
{

/**@class PolarPoint
 * A 2D point with polar coordinates.
 */

/**@brief Construct a polar point.
 */
PolarPoint::PolarPoint() {}

/**@brief Construct a polar point at the origin.
 * @param o the origin.
 */
PolarPoint::PolarPoint(const CGAL::Origin& o) : R_(0), phi_(0) {}

/**@brief Construct a clone of a polar point.
 *
 * @param p the point to clone.
 */
PolarPoint::PolarPoint(const PolarPoint& p) : R_(p.R()), phi_(p.phi()) {}

/**@brief Construct a polar point from a polar point with a different pole.
 *
 * @param p the reference polar point.
 * @param t the Cartesian coordinates of p's pole (relative to the pole of the point to construct)
 */
PolarPoint::PolarPoint(const PolarPoint& p, const Vector& t) : PolarPoint(translate_pole(p, t)) {}

/**@brief Construct a polar point.
 *
 * @param p the Cartesian coordinates of the polar point.
 */
PolarPoint::PolarPoint(const Point& p) : PolarPoint(to_polar(p)) {}

/**@brief Construct a polar point with a different pole.
 *
 * @param p the Cartesian coordinates of the polar point.
 * @param t the Cartesian coordinates of p's pole (relative to the pole of the point to construct)
 */
PolarPoint::PolarPoint(const Point& p, const Vector& t) : PolarPoint(to_polar(p + t)) {}

/**@brief Return the distance to the pole.
 * @return the distance to the pole.
 */
const Number& PolarPoint::R() const { return R_; }

/**@brief Return the angle from the pole.
 * @return the angle from the pole.
 */
const Number& PolarPoint::phi() const { return phi_; }

/**@brief Convert to a point with Cartesian coordinates.
 * @return the point with its coordinates converted to Cartesian.
 */
Point PolarPoint::to_cartesian() const
{
  const Vector d = Vector(std::cos(phi()), std::sin(phi()));
  return Point(CGAL::ORIGIN) + R() * d;
}

PolarPoint PolarPoint::to_polar(const Point& p)
{
  // Positive by construction.
  const Number R = CGAL::sqrt((p - Point(CGAL::ORIGIN)).squared_length());

  if (p.x() == 0 && p.y() == 0)
    return PolarPoint(R, 0);

  const Number phi = std::atan2(CGAL::to_double(p.y()), CGAL::to_double(p.x()));
  return PolarPoint(R, phi);
}

PolarPoint PolarPoint::translate_pole(const PolarPoint& p, const Vector& t)
{
  return to_polar(p.to_cartesian() + t);
}


bool operator==(const PolarPoint& p, const PolarPoint& q)
{
  return p.R() == q.R() && (p.R() == 0 || p.phi() == q.phi());
}

bool operator!=(const PolarPoint& p, const PolarPoint& q)
{
  return !(p == q);
}

std::ostream& operator<<(std::ostream& os, const PolarPoint& point)
{
  os << "p(R= " << point.R() << ", phi= " << point.phi() << ")";
  return os;
}

} // namespace cartocrow