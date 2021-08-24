/*
The Necklace Map library implements the algorithmic geo-visualization
method by the same name, developed by Bettina Speckmann and Kevin Verbeek
at TU Eindhoven (DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 01-04-2020
*/

#include "necklace_interval.h"

namespace cartocrow {
namespace necklace_map {

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
IntervalCentroid::IntervalCentroid(const Number& from_rad, const Number& to_rad)
    : CircularRange(from_rad, to_rad) {}

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
IntervalWedge::IntervalWedge(const Number& from_rad, const Number& to_rad)
    : CircularRange(from_rad, to_rad) {}

//Number IntervalWedge::ComputeOrder() const
//{
//  // Order based on smallest angle.
//  return from_rad();
//}

} // namespace necklace_map
} // namespace cartocrow
