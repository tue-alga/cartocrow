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

/**@class NecklaceShapeVisitor
 * @brief The base class to visit the different necklace shape types.
 *
 * This follows the visitor pattern to handle different necklace shape types in a different manner.
 */

/**@fn virtual void NecklaceShapeVisitor::Visit(CircleNecklace& shape);
 * @brief Visit a circle necklace shape.
 * @param shape the shape to visit.
 */

/**@fn virtual void NecklaceShapeVisitor::Visit(BezierNecklace& shape);
 * @brief Visit a generic necklace shape.
 * @param shape the shape to visit.
 */


/**@class NecklaceShape
 * @brief A star-shaped curve that guides the placement of data visualization symbols.
 */

/**@fn virtual const Point& NecklaceShape::kernel() const
 * @brief Give the kernel of the necklace.
 *
 * Any ray originating from this point will intersect the necklace in at most 1 point.
 * @return the kernel of the necklace.
 */

/**@fn bool NecklaceShape::IsValid() const = 0
 * @brief Check whether the necklace is valid.
 * @return whether the necklace is valid.
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

/**@brief Compute the angle of a point on the shape.
 *
 * This angle is relative to the positive x-asis.
 * @param point the point on the necklace shape.
 * @return the angle of the point in radians.
 */
Number NecklaceShape::ComputeAngleRad(const Point& point) const
{
  const Vector offset = point - kernel();

  // Note the special case where the centroid overlaps the necklace kernel.
  return
    offset.squared_length() == 0
    ? 0
    : Modulo(std::atan2(offset.y(), offset.x()));
}

/**@fn virtual void NecklaceShape::Accept(NecklaceShapeVisitor& visitor) = 0;
 * @brief Part of the visitor pattern to apply a visitor to the shape.
 * @param visitor the visitor to apply to the shape.
 */

} // namespace necklace_map
} // namespace geoviz
