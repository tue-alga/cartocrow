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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 01-04-2020
*/

#include "necklace_interval.h"


namespace geoviz
{
namespace necklace_map
{

/**@class NecklaceInterval
 * @brief The interface for a necklace interval.
 *
 * A necklace interval is a continuous interval on a circle.
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
NecklaceInterval::NecklaceInterval(const Number& from_rad, const Number& to_rad) :
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

/**@fn const Number& NecklaceInterval::from_rad() const;
 * @brief The angle where the interval starts.
 *
 * This is the clockwise extreme of the interval.
 * @return the clockwise extreme.
 */

/**@fn Number& NecklaceInterval::from_rad();
 * @brief The angle where the interval starts.
 *
 * This is the clockwise extreme of the interval.
 * @return the clockwise extreme.
 * @endparblock
 */

/**@fn const Number& NecklaceInterval::to_rad() const;
 * @brief The angle where the interval ends.
 *
 * This is the counterclockwise extreme of the interval.
 * @return the counterclockwise extreme.
 */

/**@fn Number& NecklaceInterval::to_rad();
 * @brief The angle where the interval ends.
 *
 * This is the counterclockwise extreme of the interval.
 * @return the counterclockwise extreme.
 */

/**@brief Check whether the interval is in a valid state.
 *
 * The interval is in a valid state if @f from() is in the range [0, 2*pi) and @f to() is in the range [@f from(), @f from() + 2*pi).
 * @return whether the interval is valid.
 */
bool NecklaceInterval::IsValid() const
{
  return
    0 <= from_rad() && from_rad() < M_2xPI &&
    from_rad() <= to_rad() && to_rad() < from_rad() + M_2xPI;
}

/**@brief Check whether the interval covers the full circle.
 * @return true if and only if the interval covers the full circle.
 */
bool NecklaceInterval::IsFull() const
{
  return from_rad() == 0 && to_rad() == M_2xPI;
}

/**@brief Compute a metric to order intervals on a necklace.
 *
 * This value can be compared to the ordering value of other intervals on the same necklace to imply a weak strict ordering of the ranges.
 * @return the ordering metric.
 */
//Number NecklaceInterval::ComputeOrder() const
//{
//  return 0;
//}

/**@brief Compute the angle of the centroid of the interval.
 * @return the centroid angle in radians.
 */
Number NecklaceInterval::ComputeCentroid() const
{
  return Modulo(.5 * (from() + to()));
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
 * @param from_rad the clockwise endpoint of the interval.
 * @param to_rad the counterclockwise endpoint of the interval.
 */
IntervalCentroid::IntervalCentroid(const Number& from_rad, const Number& to_rad) :
  NecklaceInterval(from_rad, to_rad)
{}

//Number IntervalCentroid::ComputeOrder() const
//{
//  // Order based on centroid.
//  const Number order = (from_rad() + to_rad()) / 2;
//  if (M_2xPI <= order)
//    return order - M_2xPI;
//  return order;
//}


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
 * @param from_rad the clockwise endpoint of the interval.
 * @param to_rad the counterclockwise endpoint of the interval.
 */
IntervalWedge::IntervalWedge(const Number& from_rad, const Number& to_rad) :
  NecklaceInterval(from_rad, to_rad)
{}

//Number IntervalWedge::ComputeOrder() const
//{
//  // Order based on smallest angle.
//  return from_rad();
//}

} // namespace necklace_map
} // namespace geoviz
