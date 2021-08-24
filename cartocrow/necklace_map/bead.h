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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 15-01-2020
*/

#ifndef CARTOCROW_NECKLACE_MAP_BEAD_H
#define CARTOCROW_NECKLACE_MAP_BEAD_H

#include <memory>

#include "cartocrow/common/circular_range.h"
#include "cartocrow/common/core_types.h"


namespace cartocrow
{
namespace necklace_map
{

struct Bead
{
  using Ptr = std::shared_ptr<Bead>;

  Bead(const Number& radius_base, const std::string& style, const std::string& id);

  bool IsValid() const;

  // Variables before scaling.
  Number radius_base;

  std::string id;  // Reference for debugging.

  CircularRange::Ptr feasible;

  std::string region_style;

  // Variables during scaling.
  Number covering_radius_rad; // TODO(tvl) move into CycleNode?

  // Variables after scaling.
  CircularRange::Ptr valid; // TODO(tvl) replace CycleNode::valid by this one and see if this can mean that the positioner needs not compute it (may have to add computing valid in fixed order scaler)?

  Number angle_rad;
}; // struct Bead

} // namespace necklace_map
} // namespace cartocrow

#endif //CARTOCROW_NECKLACE_MAP_BEAD_H
