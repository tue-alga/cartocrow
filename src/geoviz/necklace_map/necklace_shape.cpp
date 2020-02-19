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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 15-01-2020
*/

#include "necklace_shape.h"


namespace geoviz
{
namespace necklace_map
{

/**@class NecklaceShape
 * @brief A star-shaped curve that guides the placement of data visualization symbols.
 */

/**@fn virtual const Point& NecklaceShape::kernel() const
 * @brief Give the kernel of the necklace.
 *
 * Any ray originating from this point will intersect the necklace in at most 1 point.
 * @return the kernel of the necklace.
 */

/**@fn virtual bool NecklaceShape::IntersectRay(const Number& angle_rad, Point& intersection) const
 * @brief Intersect a ray originating from the kernel with the necklace.
 * @param angle_rad the counterclockwise angle between the ray and the positive x-axis in radians.
 * @param intersection the intersection of the ray with the necklace, if it exists.
 * @return @parblock a ray in the specified direction intersects the necklace.
 *
 * Note that a false return value does not indicate an invalid necklace. The necklace may not fully surround the kernel, for example a circular arc necklace covering half its supporting circle.
 * @endparblock
 */

/**@fn virtual Box NecklaceShape::ComputeBoundingBox() const = 0
 * @brief Construct a minimum bounding box of the necklace.
 * @return the minimum bounding box.
 */

/**@fn virtual Number NecklaceShape::ComputeLength() const = 0
 * @brief Compute the total length of the necklace.
 * @return the necklace length.
 */

/**@fn virtual Number NecklaceShape::ComputeRadius() const = 0
 * @brief Compute the radius of the necklace.
 *
 * Note that for non-circular necklaces, this radius will be an approximation.
 * @return the necklace radius.
 */

/**@brief Compute the angle of a point on the shape.
 *
 * This angle is relative to the positive x-asis.
 * @param point the point on the necklace shape.
 * @return the angle of the point in radians.
 */
Number NecklaceShape::ComputeAngle(const Point& point) const
{
  const Vector offset = point - kernel();

  // Note the special case where the centroid overlaps the necklace kernel.
  return
    offset.squared_length() == 0
    ? 0
    : std::atan2(offset.y(), offset.x());
}

/**@fn virtual void NecklaceShape::Accept(NecklaceShapeVisitor& visitor) = 0;
 * @brief Part of the visitor pattern to apply a visitor to the shape.
 * @param visitor the visitor to apply to the shape.
 */


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

Number CircleNecklace::ComputeLength() const
{
  return length_;
}

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
CurveNecklace::CurveNecklace(const Circle& shape, const Number& angle_cw_rad, const Number& angle_ccw_rad) :
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
}


/**@class GenericNecklace
 * @brief A generic star-shaped necklace.
 *
 * Note that for this necklace, the kernel must be set explicitly.
 */


/**@class NecklaceShapeVisitor
 * @brief The base class to visit the different necklace shape types.
 *
 * This follows the visitor pattern to handle different necklace shape types in a different manner.
 */

/**@fn virtual void NecklaceShapeVisitor::Visit(CircleNecklace& shape);
 * @brief Visit a circle necklace shape.
 * @param shape the shape to visit.
 */

/**@fn virtual void NecklaceShapeVisitor::Visit(CurveNecklace& shape);
 * @brief Visit a circular curve necklace shape.
 * @param shape the shape to visit.
 */

/**@fn virtual void NecklaceShapeVisitor::Visit(GenericNecklace& shape);
 * @brief Visit a generic necklace shape.
 * @param shape the shape to visit.
 */

} // namespace necklace_map
} // namespace geoviz
