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

#include <glog/logging.h>


namespace geoviz
{
namespace necklace_map
{

/**@class NecklaceShape
 * @brief A star-shaped curve that guides the placement of data visualization glyphs.
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
  : NecklaceShape(), shape_(shape) {}

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
  const Number radius = CGAL::sqrt(shape_.squared_radius());
  return M_2xPI * radius;
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
  if (angle_cw_rad_ == angle_ccw_rad_)
  {
    // If the necklace covers the whole circle, we set the angles to 0 for convenience.
    angle_cw_rad_ = angle_ccw_rad_ = 0;
    return;
  }

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


/**@class NecklaceInterval
 * @brief A connected part of a necklace.
 */

/**@brief Construct a necklace interval.
 *
 * The interval covers the intersection of the necklace and a wedge with its apex at the necklace kernel. This wedge is bounded by two rays from the center, which are described by their angle relative to the positive x axis in counterclockwise direction.
 *
 * The order of these rays is important: the interval is used that lies counterclockwise relative to the first angle.
 *
 * If the rays are identical, the interval covers the full necklace. This should not occur in practice.
 *
 * Note that the interval includes the intersections of the wedge boundaries with the necklace, i.e. the interval is closed.
 * @param angle_cw_rad the clockwise endpoint of the interval.
 * @param angle_ccw_rad the counterclockwise endpoint of the interval.
 */
NecklaceInterval::NecklaceInterval(const Number& angle_cw_rad, const Number& angle_ccw_rad)
  : angle_cw_rad_(angle_cw_rad), angle_ccw_rad_(angle_ccw_rad)
{
  if (angle_cw_rad_ == angle_ccw_rad_)
  {
    // If the interval covers the whole necklace, we set the angles to 0 for convenience.
    angle_cw_rad_ = angle_ccw_rad_ = 0;
    return;
  }

  while (angle_cw_rad_ < 0)
    angle_cw_rad_ += M_2xPI;
  while (M_2xPI <= angle_cw_rad_)
    angle_cw_rad_ -= M_2xPI;

  while (angle_ccw_rad_ <= angle_cw_rad_)
    angle_ccw_rad_ += M_2xPI;
  while (angle_cw_rad_ + M_2xPI < angle_ccw_rad_)
    angle_ccw_rad_ -= M_2xPI;
}

/**@brief The angle where the interval ends when traversing the interval in clockwise direction.
 *
 * @return @parblock the clockwise ending angle.
 *
 * This angle is guaranteed to be in the range [0, 2*pi).
 * @endparblock
 */
const Number& NecklaceInterval::angle_cw_rad() const { return angle_cw_rad_; }

/**@brief The angle where the interval ends when traversing the interval in counterclockwise direction.
 *
 * @return @parblock the counterclockwise ending angle.
 *
 * This angle is guaranteed to be in the range [angle_cw_rad, angle_cw_rad+2*pi).
 * @endparblock
 */
const Number& NecklaceInterval::angle_ccw_rad() const { return angle_ccw_rad_; }

/**@brief Check whether the interval is in a valid state.
 * @return whether the interval is valid.
 */
bool NecklaceInterval::IsValid() const
{
  return
    0 <= angle_cw_rad_ && angle_cw_rad_ < M_2xPI &&
    angle_cw_rad_ <= angle_ccw_rad_ && angle_ccw_rad_ < angle_cw_rad_ + M_2xPI;
}

/**@brief Check whether the interval intersects a ray from the necklace kernel.
 * @param angle_rad @parblock the counterclockwise angle between the ray and the positive x axis.
 *
 * This angle must be in the range [0, 2*pi).
 * @endparblock
 * @return whether the interval contains the intersection of the ray with the necklace.
 */
bool NecklaceInterval::IntersectsRay(const Number& angle_rad) const
{
  CHECK_GE(angle_rad, 0);
  CHECK_LT(angle_rad, M_2xPI);
  return
    angle_cw_rad_ == angle_ccw_rad_ ||
    (angle_rad < angle_cw_rad_ ? angle_rad + M_2xPI : angle_rad) < angle_ccw_rad_;
}


/**@class IntervalCentroid
 * @brief A centroid-based necklace interval.
 */

/**@brief Construct a centroid interval.
 *
 * The interval covers the intersection of the necklace and a wedge with its apex at the necklace kernel. This wedge is bounded by two rays from the center, which are described by their angle relative to the positive x axis in counterclockwise direction.
 *
 * The order of these rays is important: the interval is used that lies counterclockwise relative to the first angle.
 *
 * If the rays are identical, the interval covers the full necklace. This should not occur in practice.
 *
 * Note that the interval includes the intersections of the wedge boundaries with the necklace, i.e. the interval is closed.
 * @param angle_cw_rad the clockwise endpoint of the interval.
 * @param angle_ccw_rad the counterclockwise endpoint of the interval.
 */
IntervalCentroid::IntervalCentroid(const Number& angle_cw_rad, const Number& angle_ccw_rad)
  : NecklaceInterval(angle_cw_rad, angle_ccw_rad) {}

Number IntervalCentroid::ComputeOrder() const
{
  // Order based on centroid.
  const Number order = (angle_cw_rad_ + angle_ccw_rad_) / 2;
  if (M_2xPI <= order)
    return order - M_2xPI;
  return order;
}


/**@class IntervalWedge
 * @brief A wedge-based necklace interval.
 */

/**@brief Construct a wedge interval.
 *
 * The interval covers the intersection of the necklace and a wedge with its apex at the necklace kernel. This wedge is bounded by two rays from the center, which are described by their angle relative to the positive x axis in counterclockwise direction.
 *
 * The order of these rays is important: the interval is used that lies counterclockwise relative to the first angle.
 *
 * If the rays are identical, the interval covers the full necklace. This should not occur in practice.
 *
 * Note that the interval includes the intersections of the wedge boundaries with the necklace, i.e. the interval is closed.
 * @param angle_cw_rad the clockwise endpoint of the interval.
 * @param angle_ccw_rad the counterclockwise endpoint of the interval.
 */
IntervalWedge::IntervalWedge(const Number& angle_cw_rad, const Number& angle_ccw_rad)
  : NecklaceInterval(angle_cw_rad, angle_ccw_rad) {}

Number IntervalWedge::ComputeOrder() const
{
  // Order based on smallest angle.
  return angle_cw_rad_;
}


/**@struct Necklace
 * @brief A collection of visualization glyphs that are organized on a curve.
 */

/**@brief Construct a necklace from a shape.
 * @param shape the shape of the necklace.
 */
Necklace::Necklace(const NecklaceShape::Ptr& shape) : shape(shape)/*, glyph()*/ {}

/**@fn NecklaceShape::Ptr NecklaceShape::shape
 * @brief the shape used to organize the glyphs.
 */

/**@fn NecklaceGlyph::Ptr NecklaceShape::glyph
 * @brief one of the glyphs on the necklace.
 *
 * Note that the other glyphs can be reached by traversing through glyph->next.
 */


/**@struct NecklaceGlyph
 * @brief A visualization element to show the numeric value associated with a map region.
 *
 * Each region with a value larger than 0 that is also assigned a necklace will get a necklace glyph. The value is visualized using the area of the element. While this does not directly convey the absolute value, the difference between glyphs exposes their relative values.
 *
 * While glyphs could have various shapes, we currently only support disks.
 */

/**@brief Construct a necklace glyph.
 * @param radius_base the radius of the glyph before scaling.
 */
NecklaceGlyph::NecklaceGlyph(const Necklace::Ptr necklace)
  : necklace(necklace), interval(), angle_rad(0), angle_min_rad(0), angle_max_rad(0) {}

/**@brief Check whether the glyph is valid.
 *
 * This validity depends on three aspects: the interval must not be null, the interval must be valid, and the glyph's position must be in the interval.
 */
bool NecklaceGlyph::IsValid() const
{
  return necklace != nullptr && interval != nullptr && interval->IsValid() && interval->IntersectsRay(angle_rad);
}

} // namespace necklace_map
} // namespace geoviz