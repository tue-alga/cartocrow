/*
The GeoViz library implements algorithmic geo-visualization methods,
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 06-05-2020
*/

#include "circular_range.h"


namespace geoviz
{

/**@class CircularRange
 * @brief The interface for a necklace interval.
 *
 * A necklace interval is a continuous interval on a circle.
 */

/**@fn CircularRange::Ptr
 * @brief The preferred pointer type for storing or sharing a circular range.
 */

/**@brief Construct an interval.
 *
 * The interval covers the intersection of the necklace and a wedge with its apex at the necklace kernel. This wedge is bounded by two rays from the center, which are described by their angle relative to the positive x axis in counterclockwise direction.
 *
 * The order of these rays is important: the interval is used that lies counterclockwise relative to the first angle.
 *
 * If the rays are identical, the interval covers a single point. If to_rad is at least 2*pi larger than from_rad, the interval covers the full circle.
 * @param from_rad the clockwise endpoint of the interval.
 * @param to_rad the counterclockwise endpoint of the interval.
 */
CircularRange::CircularRange(const Number& from_rad, const Number& to_rad) :
  Range(from_rad, to_rad)
{
  if (M_2xPI <= to_rad - from_rad)
  {
    from() = 0;
    to() = M_2xPI;
  }
  else
  {
    from() = Modulo(from_rad);
    to() = Modulo(to_rad, from());
  }
}

/**@brief Construct a circular range from a regular range.
 *
 * The circular range covers the same interval on the circle as the given range.
 * @param range the range to base the circular range on.
 */
CircularRange::CircularRange(const Range& range) : CircularRange(range.from(), range.to()) {}

/**@fn const Number& CircularRange::from_rad() const;
 * @brief The angle where the interval starts.
 *
 * This is the clockwise extreme of the interval.
 * @return the clockwise extreme.
 */

/**@fn Number& CircularRange::from_rad();
 * @brief The angle where the interval starts.
 *
 * This is the clockwise extreme of the interval.
 * @return the clockwise extreme.
 */

/**@fn const Number& CircularRange::to_rad() const;
 * @brief The angle where the interval ends.
 *
 * This is the counterclockwise extreme of the interval.
 * @return the counterclockwise extreme.
 */

/**@fn Number& CircularRange::to_rad();
 * @brief The angle where the interval ends.
 *
 * This is the counterclockwise extreme of the interval.
 * @return the counterclockwise extreme.
 */

/**@brief Check whether the interval is in a valid state.
 *
 * The interval is in a valid state if from() is in the range [0, 2*pi) and to() is in the range [from(), from() + 2*pi).
 * @return whether the interval is valid.
 */
bool CircularRange::IsValid() const
{
  return
    0 <= from_rad() && from_rad() < M_2xPI &&
    from_rad() <= to_rad() && to_rad() < from_rad() + M_2xPI;
}

/**@brief Check whether the interval covers the full circle.
 * @return true if and only if the interval covers the full circle.
 */
bool CircularRange::IsFull() const
{
  return from_rad() == 0 && to_rad() == M_2xPI;
}

bool CircularRange::Contains(const Number& value) const
{
  const Number value_mod = Modulo(value, from_rad());
  return from_rad() <= value_mod && value_mod <= to_rad();
}

bool CircularRange::ContainsOpen(const Number& value) const
{
  const Number value_mod = Modulo(value, from_rad());
  return from_rad() < value_mod && value_mod < to_rad();
}

bool CircularRange::Intersects(const Range::Ptr& range) const
{
  CircularRange interval(*range);
  return
    Contains(interval.from_rad()) ||
    interval.Contains(from_rad());
}

bool CircularRange::IntersectsOpen(const Range::Ptr& range) const
{
  CircularRange interval(*range);
  return
    (Contains(interval.from_rad()) && Modulo(interval.from_rad(), from_rad()) != to_rad()) ||
    (interval.Contains(from_rad()) && Modulo(from_rad(), interval.from_rad()) != interval.to());
}

/**@brief Compute a metric to order intervals on a necklace.
 *
 * This value can be compared to the ordering value of other intervals on the same necklace to imply a weak strict ordering of the ranges.
 * @return the ordering metric.
 */
//Number CircularRange::ComputeOrder() const
//{
//  return 0;
//}

/**@brief Compute the angle of the centroid of the interval.
 * @return the centroid angle in radians.
 */
Number CircularRange::ComputeCentroid() const
{
  return Modulo(.5 * (from() + to()));
}

/**@brief Reverse the orientation of the range.
 */
void CircularRange::Reverse()
{
  if (IsFull())
    return;

  const Number from_rad_old = from_rad();
  from_rad() = Modulo(to_rad());
  to_rad() = Modulo(from_rad_old, from_rad());
}

} // namespace geoviz
