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
 * @brief Check whether the shape is valid.
 * @return whether the shape is valid.
 */

/**@fn bool NecklaceShape::IsEmpty() const = 0
 * @brief Check whether the shape is empty.
 * @return whether the shape is valid, i.e. it covers an empty region.
 */

/**@fn bool NecklaceShape::IsClosed() const = 0
 * @brief Check whether the shape is closed.
 * @return whether the shape is closed.
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

/**@fn virtual Number NecklaceShape::ComputeCoveringRadiusRad(const Range::Ptr& range, const Number& radius) const = 0
 * @brief Compute the covering radius based on a bead of a given radius that is centered in a given range.
 *
 * For circle necklaces, this covering radius is the part of the circle covered by the smallest wedge with the necklace kernel as apex and containing the bead. This covering radius does not actually depend on the range, because of the regularity of the circle.
 *
 * For bezier curves, this covering radius is based on the points at which the curve intersects the bead boundary. The largest covering radius is taken over the range of bead centers.
 * @param range the range on which the bead can be centered.
 * @param radius the radius of the bead.
 * @return @parblock the covering radius in radians.
 *
 * Note that this covering radius is always an arc measurement.
 * @endparblock
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

/**@fn virtual Number NecklaceShape::ComputeAngleAtDistanceRad(const Number& angle_rad, const Number& distance) const = 0
 * @brief Compute the angle difference between a point on the necklace at a given angle and another point on the necklace that is a given geodetic distance away from the point.
 *
 * This distance is always traveled in counterclockwise direction.
 * @param angle_rad the angle relative to the positive x-axis of the ray originating from the necklace kernel that intersects the necklace in the original point.
 * @param distance the distance from the original point to travel (along the necklace) before determining the final angle.
 * @return the angle relative to the positive x-axis of the ray originating from the necklace kernel that intersects the necklace in the final point.
 */

/**@fn virtual void NecklaceShape::Accept(NecklaceShapeVisitor& visitor) = 0;
 * @brief Part of the visitor pattern to apply a visitor to the shape.
 * @param visitor the visitor to apply to the shape.
 */

} // namespace necklace_map
} // namespace geoviz
