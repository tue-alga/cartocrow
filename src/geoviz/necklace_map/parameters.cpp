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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 28-01-2020
*/

#include "parameters.h"


namespace geoviz
{
namespace necklace_map
{

/**@class geoviz::necklace_map::IntervalType
 * @brief A type of feasible interval on a necklace.
 */

/**@class geoviz::necklace_map::OrderType
 * @brief A type of ordering to apply when computing the optimal scale factor and bead placement.
 */


/**@struct Parameters
 * @brief A struct to collect the parameters used for computing the necklace map.
 *
 * These parameters include those needed for computing the feasible intervals, the optimal scale factor, and a valid placement for the necklace beads.
 */

/**@brief Construct a collection of parameters;
 *
 * All parameters are initialized as valid values.
 */
Parameters::Parameters() :
  interval_type(IntervalType::kCentroid),
  centroid_interval_length_rad(1),
  ignore_point_regions(false),
  order_type(OrderType::kFixed),
  buffer_rad(0),
  aversion_ratio(0)
{}

/**@fn IntervalType Parameters::interval_type;
 * @brief The type of feasible intervals to compute.
 */

/**@fn Number Parameters::centroid_interval_length_rad;
 * @brief The length of any centroid intervals generated when computing the feasible intervals.
 */

/**@fn bool Parameters::ignore_point_regions;
 * @brief Whether to ignore degenerate (point) regions.
 *
 * Point regions that are not ignored are always assigned a centroid region.
 */

/**@fn OrderType Parameters::order_type;
 * @brief The type of order imposed on the necklace beads.
 *
 * This order is used when computing the optimal scale factor and when computing a valid placement.
 */

/**@fn Number Parameters::buffer_rad;
 * @brief The minimum angle in radians of the empty wedge between neighboring necklace beads that has the necklace kernel as apex.
 *
 * This buffer is used when computing the optimal scale factor and when computing a valid placement.
 */

/**@fn Number Parameters::aversion_ratio;
 * @brief The ratio between attraction to the interval center (0) and repulsion from the neighboring beads (1).
 *
 * This ratio must be in the range (0, 1].
 */

} // namespace necklace_map
} // namespace geoviz
