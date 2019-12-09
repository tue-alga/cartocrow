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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 07-11-2019
*/

#include "necklace.h"


namespace geoviz
{
namespace necklace_map
{

/**@class NecklaceType
 * @brief A necklace is a star-shaped curve that guides the placement of data visualization glyphs.
 */

/**@fn virtual const Point& NecklaceType::kernel() const
 * @brief Give the kernel of the necklace.
 *
 * Any ray originating from this point will intersect the necklace in at most 1 point.
 * @return the kernel of the necklace.
 */

/**@fn virtual bool NecklaceType::IntersectRay(const Number& angle_rad, Point& intersection) const
 * @brief Intersect a ray originating from the kernel with the necklace.
 * @param angle_rad the counterclockwise angle between the ray and the positive x-axis in radians.
 * @param intersection the intersection of the ray with the necklace, if it exists.
 * @return @parblock a ray in the specified direction intersects the necklace.
 *
 * Note that a false return value does not indicate an invalid necklace. The necklace may not fully surround the kernel, for example a circular arc necklace covering half its supporting circle.
 * @endparblock
 */

/**@fn virtual Box NecklaceType::ComputeBoundingBox() const = 0
 * @brief Construct a minimum bounding box of the necklace.
 * @return the minimum bounding box.
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
CircleNecklace::CircleNecklace(const Circle& shape)
  : NecklaceType(), shape_(shape) {}

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


/**@class CurveNecklace
 * @brief A circular arc necklace.
 *
 * Note that the circular arc may cover the full circle. In this case, the glyph placement will be identical to the placement on a CircleNecklace of the supporting circle.
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
CurveNecklace::CurveNecklace(const Circle& shape, const Number& angle_cw_rad, const Number& angle_ccw_rad)
  : CircleNecklace(shape),
    angle_cw_rad_(angle_cw_rad),
    angle_ccw_rad_(angle_ccw_rad)
{
  while (angle_cw_rad_ < 0)
    angle_cw_rad_ += M_2xPI;
  while (M_2xPI <= angle_cw_rad_)
    angle_cw_rad_ -= M_2xPI;

  while (angle_ccw_rad_ <= angle_cw_rad_)
    angle_ccw_rad_ += M_2xPI;
  while (angle_cw_rad_ + M_2xPI < angle_ccw_rad_)
    angle_ccw_rad_ -= M_2xPI;
}

bool CurveNecklace::IntersectRay(const Number& angle_rad, Point& intersection) const
{
  // Check whether the angle lies in the covered part of the supporting circle.
  if
  (
    angle_ccw_rad_ < angle_rad ||
    (angle_rad < angle_cw_rad_ && angle_ccw_rad_ < angle_rad + M_2xPI)
  )
    return false;

  return CircleNecklace::IntersectRay(angle_rad, intersection);
}


/**@class GenericNecklace
 * @brief A generic star-shaped necklace.
 *
 * Note that for this necklace, the kernel must be set explicitly.
 */

} // namespace necklace_map
} // namespace geoviz