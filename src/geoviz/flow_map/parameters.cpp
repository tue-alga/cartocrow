/*
The Flow Map library implements the algorithmic geo-visualization
method by the same name, developed by Kevin Verbeek, Kevin Buchin,
and Bettina Speckmann at TU Eindhoven
(DOI: 10.1007/s00453-013-9867-z & 10.1109/TVCG.2011.202).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 02-09-2020
*/

#include "parameters.h"


namespace geoviz
{
namespace flow_map
{

/**@struct Parameters
 * @brief A struct to collect the parameters used for computing the flow map.
 */

/**@brief Construct a collection of parameters;
 *
 * All parameters are initialized as valid values.
 */
Parameters::Parameters() :
  restricting_angle_rad(0.61)
{}

/**@fn Number Parameters::restricting_angle_rad;
 * @brief The maximum angle between the line connecting the root and any point on a tree arc and arc's tangent line at that point.
 *
 * This angle must be in the range (0, pi/2).
 *
 * In practice, all arcs of a spiral tree are either straight lines or alpha-spirals, where alpha is this restriction angle.
 *
 * These spirals can be expressed as polar coordinates around the root node p(t) = (R(t), phi(t)), where R = R(0)*e^{-t} and phi(t) = phi(0) + tan(alpha)*t.
 *
 * Note that generally p(0) is the polar coordinates of a node of a spiral arc, positive t values are closer to the root, and negative t values are farther from the root.
 */

} // namespace flow_map
} // namespace geoviz
