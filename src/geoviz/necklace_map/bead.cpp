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

#include "bead.h"

#include <glog/logging.h>


namespace geoviz
{
namespace necklace_map
{

/**@struct Bead
 * @brief A visualization element to show the numeric value associated with a map region.
 *
 * Each region with a value larger than 0 that is also assigned a necklace will get a bead on the necklace. The value is visualized using the area of the bead. While this does not directly convey the absolute value, the relative difference between beads exposes the relative values of their associated regions.
 *
 * While beads could have various shapes, we currently only support disks.
 */

/**@brief Construct a necklace bead.
 * @param radius_base the unscaled radius of the bead.
 */
Bead::Bead(const Number& radius_base)
  : radius_base(radius_base), feasible(), covering_radius_scaled_rad(-1), valid(), angle_rad(0) {}

/**@brief Check whether the bead is valid.
 *
 * This validity depends on three aspects: the feasible interval must not be null, the interval must be valid, and the bead's position must be in the feasible interval.
 *
 * Note that this check does not take into account overlap with other beads.
 */
bool Bead::IsValid() const
{
  CHECK(false) << "Implementation not checked";
  return feasible != nullptr && feasible->IsValid() && feasible->IntersectsRay(angle_rad);
}

} // namespace necklace_map
} // namespace geoviz
