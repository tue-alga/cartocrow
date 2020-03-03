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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 09-12-2019
*/

#include "range.h"

#include <glog/logging.h>


namespace geoviz
{
namespace necklace_map
{

/**@class CircleRange
 * @brief A continuous interval on a circle.
 */

/**@brief Constrain a value to be beyond some starting value by at most 2*pi.
 * @param value the value (in radians) to constrain.
 * @param start the minimum value in radians.
 * @return the value in the constrained range.
 */
Number CircleRange::Modulo(const Number& value_rad, const Number& start_rad /*= 0*/)
{
  Number constrained_rad = value_rad;
  while (constrained_rad < start_rad)
    constrained_rad += M_2xPI;
  while (start_rad + M_2xPI <= constrained_rad)
    constrained_rad -= M_2xPI;
  return constrained_rad;
}

/**@brief Construct a range on a circle.
 *
 * The interval covers the intersection of the necklace and a wedge with its apex at the necklace kernel. This wedge is bounded by two rays from the center, which are described by their angle relative to the positive x axis in counterclockwise direction.
 *
 * The order of these rays is important: the interval is used that lies counterclockwise relative to the first angle.
 *
 * If the rays are identical, the interval covers a single point. If the rays are exactly 0 and 2*pi, the interval covers the full circle. This should not occur in practice.
 *
 * Note that the interval includes the intersections of the wedge boundaries with the necklace, i.e. the interval is closed.
 * @param angle_cw_rad the clockwise endpoint of the interval.
 * @param angle_ccw_rad the counterclockwise endpoint of the interval.
 */
CircleRange::CircleRange(const Number& angle_cw_rad, const Number& angle_ccw_rad)
  : angle_cw_rad_(angle_cw_rad), angle_ccw_rad_(angle_ccw_rad)
{
  if (angle_cw_rad_ == angle_ccw_rad_)
  {
    angle_cw_rad_ = angle_ccw_rad_ = Modulo(angle_cw_rad_);
  }
  else
  {
    angle_cw_rad_ = Modulo(angle_cw_rad_);
    angle_ccw_rad_ = Modulo(angle_ccw_rad_, angle_cw_rad_);
  }
}

/**@brief Copy a range on a circle.
 * @param range the range to copy.
 */
CircleRange::CircleRange(const CircleRange& range) :
  angle_cw_rad_(range.angle_cw_rad_), angle_ccw_rad_(range.angle_ccw_rad_)
{}

/**@brief The angle where the interval ends when traversing the interval in clockwise direction.
 *
 * @return @parblock the clockwise ending angle.
 *
 * This angle is guaranteed to be in the range [0, 2*pi).
 * @endparblock
 */
const Number& CircleRange::angle_cw_rad() const { return angle_cw_rad_; }

/**@brief The angle where the interval ends when traversing the interval in clockwise direction.
 *
 * @return @parblock the clockwise ending angle.
 *
 * This angle is guaranteed to be in the range [0, 2*pi).
 * @endparblock
 */
Number& CircleRange::angle_cw_rad() { return angle_cw_rad_; }

/**@brief The angle where the interval ends when traversing the interval in counterclockwise direction.
 *
 * @return @parblock the counterclockwise ending angle.
 *
 * This angle is guaranteed to be in the range [angle_cw_rad, angle_cw_rad+2*pi).
 * @endparblock
 */
const Number& CircleRange::angle_ccw_rad() const { return angle_ccw_rad_; }

/**@brief The angle where the interval ends when traversing the interval in counterclockwise direction.
 *
 * @return @parblock the counterclockwise ending angle.
 *
 * This angle is guaranteed to be in the range [angle_cw_rad, angle_cw_rad+2*pi).
 * @endparblock
 */
Number& CircleRange::angle_ccw_rad() { return angle_ccw_rad_; }

/**@brief Check whether the interval is in a valid state.
 * @return whether the interval is valid.
 */
bool CircleRange::IsValid() const
{
  if (angle_cw_rad_ == M_2xPI)
    return angle_cw_rad_ == 0;

  return
    0 <= angle_cw_rad_ && angle_cw_rad_ < M_2xPI &&
    0 <= angle_ccw_rad_ && angle_ccw_rad_ < M_2xPI;
}

/**@brief Check whether the range is degenerate.
 * @return true if the range is a single point/
 */
bool CircleRange::IsDegenerate() const
{
  return angle_cw_rad_ == angle_ccw_rad_;
}

/**@brief Check whether the range covers the full circle.
 * @return true if and only if the range covers the full circle.
 */
bool CircleRange::IsCircle() const
{
  return angle_cw_rad_ == 0 && angle_ccw_rad_ == M_2xPI;
}

/**@brief Check whether the interval intersects a ray from the necklace kernel.
 * @param angle_rad @parblock the counterclockwise angle between the ray and the positive x axis.
 *
 * This angle must be in the range [0, 2*pi).
 * @endparblock
 * @return whether the interval contains the intersection of the ray with the necklace.
 */
bool CircleRange::IntersectsRay(const Number& angle_rad) const
{
  CHECK_GE(angle_rad, 0);
  CHECK_LT(angle_rad, M_2xPI);
  return
    angle_cw_rad_ <= angle_ccw_rad_
    ? (angle_cw_rad_ <= angle_rad && angle_rad <= angle_ccw_rad_)
    : (angle_rad <= angle_ccw_rad_ || angle_cw_rad_ <= angle_rad);
}

/**@brief Check whether this range and another range intersect in their interior.
 * @param range the range for which to check the intersection.
 * @return whether the ranges intersect in their interior.
 */
bool CircleRange::Intersects(const CircleRange::Ptr& range) const
{
  return
    (range->IntersectsRay(from_rad()) && from_rad() != range->to_rad()) ||
    (IntersectsRay(range->from_rad()) && range->from_rad() != to_rad());
}

/**@brief Compute the angle of the centroid of the range.
 * @return the centroid angle in radians.
 */
Number CircleRange::ComputeCentroid() const
{
  const Number centroid = .5 * (angle_cw_rad() + angle_ccw_rad());
  CHECK_GE(centroid, 0);
  return angle_cw_rad_ < angle_ccw_rad_ ? centroid : centroid + M_PI;
}

/**@brief Compute the total length of the range.
 * @return the total length in radians.
 */
Number CircleRange::ComputeLength() const
{
  const Number length = angle_ccw_rad() - angle_cw_rad();
  return Modulo(length);
}

/**@brief Compute a metric to order different ranges on the same necklace.
 *
 * This value can be compared to the ordering value of other ranges on the same necklace to imply a strict ordering of the ranges.
 * @return the ordering metric.
 */
Number CircleRange::ComputeOrder() const
{
  return 0;
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
  : CircleRange(angle_cw_rad, angle_ccw_rad) {}

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
  : CircleRange(angle_cw_rad, angle_ccw_rad) {}

Number IntervalWedge::ComputeOrder() const
{
  // Order based on smallest angle.
  return angle_cw_rad_;
}

} // namespace necklace_map
} // namespace geoviz
