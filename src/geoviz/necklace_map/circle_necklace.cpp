/*
The Necklace Map library implements the algorithmic geo-visualization
method by the same name, developed by Bettina Speckmann and Kevin Verbeek
at TU Eindhoven (DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
Copyright (C) 2019  Netherlands eScience Center and TU Eindhoven

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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 25-02-2020
*/

#include "circle_necklace.h"


namespace geoviz
{
namespace necklace_map
{

/**@class CircleNecklace
 * @brief A full circle necklace.
 */

/**@brief Construct a circle necklace.
 *
 * The necklace kernel is the circle center.
 *
 * @param shape the circle covered by the necklace.
 */
CircleNecklace::CircleNecklace(const Circle& shape) : NecklaceShape(), shape_(shape)
{
  radius_ = CGAL::sqrt(shape_.squared_radius());
  length_ = M_2xPI * radius_;
}

const Point& CircleNecklace::kernel() const
{
  return shape_.center();
}

bool CircleNecklace::IntersectRay(const Number& angle_rad, Point& intersection) const
{
  const Vector relative = CGAL::sqrt(shape_.squared_radius()) * Vector( std::cos(angle_rad), std::sin(angle_rad) );
  intersection = kernel() + relative;
  return true;
}

Box CircleNecklace::ComputeBoundingBox() const
{
  const Number radius = CGAL::sqrt(shape_.squared_radius());
  return Box
    (
      shape_.center().x() - radius,
      shape_.center().y() - radius,
      shape_.center().x() + radius,
      shape_.center().y() + radius
    );
}

/*Number CircleNecklace::ComputeLength() const
{
  return length_;
}*/

Number CircleNecklace::ComputeRadius() const
{
  return radius_;
}

void CircleNecklace::Accept(NecklaceShapeVisitor& visitor)
{
  visitor.Visit(*this);
}


/**@class CurveNecklace
 * @brief A circular arc necklace.
 *
 * Note that the circular arc may cover the full circle. In this case, the bead placement will be identical to the placement on a CircleNecklace of the supporting circle.
 */

/**@brief Construct a curve necklace.
 *
 * The curve covers the intersection of the circle and a wedge with its apex at the circle center. This wedge is bounded by two rays from the center, which are described by their angle relative to the positive x axis in counterclockwise direction.
 *
 * The order of these rays is important: the wedge is used that lies counterclockwise relative to the first angle.
 *
 * If the rays are identical, the necklace covers the full circle.
 *
 * Note that the necklace includes the intersections of the wedge boundaries with the circle, i.e. the curve is closed.
 *
 * The necklace kernel is the circle center.
 * @param shape the supporting circle of the necklace.
 * @param angle_cw_rad the clockwise endpoint of the curve.
 * @param angle_ccw_rad the counterclockwise endpoint of the curve.
 */
/*CurveNecklace::CurveNecklace(const Circle& shape, const Number& angle_cw_rad, const Number& angle_ccw_rad) :
  CircleNecklace(shape),
  interval_(angle_cw_rad, angle_ccw_rad)
{}

bool CurveNecklace::IntersectRay(const Number& angle_rad, Point& intersection) const
{
  // Check whether the angle lies in the covered part of the supporting circle.
  if (!interval_.IntersectsRay(angle_rad))
    return false;

  return CircleNecklace::IntersectRay(angle_rad, intersection);
}

void CurveNecklace::Accept(NecklaceShapeVisitor& visitor)
{
  visitor.Visit(*this);
}*/

} // namespace necklace_map
} // namespace geoviz
