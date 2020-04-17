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

#ifndef GEOVIZ_NECKLACE_MAP_BEAD_H
#define GEOVIZ_NECKLACE_MAP_BEAD_H

#include <memory>

#include "geoviz/common/core_types.h"
#include "geoviz/necklace_map/necklace_interval.h"


namespace geoviz
{
namespace necklace_map
{

struct Bead
{
  using Ptr = std::shared_ptr<Bead>;

  Bead(const Number& radius_base, const std::string& style, const std::string& id/*tmp debug*/);

  bool IsValid() const;

  // Variables before scaling.
  Number radius_base;  // TODO(tvl) express the radius in radians.

  NecklaceInterval::Ptr feasible;

  std::string region_style;
  std::string id; // TODO(tvl) tmp for debugging.

  // Variables during scaling.
  Number covering_radius_rad; // TODO(tvl) move into CycleNode?

  int check; // TODO(tvl) move into AnyOrderCycleNode?

  // Variables after scaling.
  NecklaceInterval::Ptr valid; // TODO(tvl) replace CycleNode::valid by this one and see if this can mean that the positioner needs not compute it (may have to add computing valid in fixed order scaler)?

  Number angle_rad;
}; // struct Bead

} // namespace necklace_map
} // namespace geoviz

#endif //GEOVIZ_NECKLACE_MAP_BEAD_H
