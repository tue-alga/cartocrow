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

/**@struct Necklace
 * @brief A collection of visualization symbols that are organized on a curve.
 */

/**@fn Necklace::Ptr
 * @brief The preferred pointer type for storing or sharing a necklace.
 */

/**@brief Construct a necklace from a shape.
 * @param shape the shape of the necklace.
 */
Necklace::Necklace(const NecklaceShape::Ptr& shape) : shape(shape), beads() {}

/**@brief Sort the beads of the necklace by the clockwise extremes of their feasible interval.
 */
void Necklace::SortBeads()
{
  for (const Bead::Ptr& bead : beads)
    CHECK_NOTNULL(bead);

  // Sort the beads by the clockwise extreme of their feasible interval.
  std::sort
  (
    beads.begin(),
    beads.end(),
    [](const Bead::Ptr& a, const Bead::Ptr& b)
    {
      return a->feasible->from_rad() < b->feasible->from_rad();
    }
  );
}

/**@fn NecklaceShape::Ptr Necklace::shape;
 * @brief The shape of the necklace.
 */

/**@fn std::vector<Bead::Ptr> Necklace::beads;
 * @brief The beads on the necklace.
 */

} // namespace necklace_map
} // namespace geoviz
