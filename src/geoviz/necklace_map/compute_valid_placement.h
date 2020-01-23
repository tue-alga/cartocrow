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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 07-01-2020
*/

#ifndef GEOVIZ_NECKLACE_MAP_COMPUTE_VALID_PLACEMENT_H
#define GEOVIZ_NECKLACE_MAP_COMPUTE_VALID_PLACEMENT_H

#include <vector>

#include "geoviz/common/core_types.h"
#include "geoviz/necklace_map/necklace.h"
#include "geoviz/necklace_map/detail/validate_scale_factor.h"


namespace geoviz
{
namespace necklace_map
{
// TODO(tvl) Add a note on multiple necklaces that the different necklaces may generate overlapping beads; these can often be corrected by manually tuning the buffer and attraction-repulsion parameters. We don't fix this, or even check for occurrence of overlapping beads.

struct ComputeValidPlacement
{
  ComputeValidPlacement(const Number& scale_factor, const Number& aversion_ratio, const Number& buffer_rad = 0);

  void operator()(Necklace::Ptr& necklace) const;

  void operator()(std::vector<Necklace::Ptr>& necklaces) const;

  Number scale_factor;
  Number aversion_ratio;  // Ratio between attraction to interval center (0) and repulsion from neighboring beads (1).
  Number buffer_rad;

 protected:
  virtual void SwapBeads(Necklace::Ptr& necklace) const = 0;
}; // struct ComputeValidPlacement

struct ComputeValidPlacementFixedOrder : public ComputeValidPlacement
{
  ComputeValidPlacementFixedOrder(const Number& scale_factor, const Number& aversion_ratio, const Number& min_separation = 0);

 protected:
  void SwapBeads(Necklace::Ptr& necklace) const {}
}; // struct ComputeValidPlacementFixedOrder

struct ComputeValidPlacementAnyOrder : public ComputeValidPlacement
{
  ComputeValidPlacementAnyOrder(const Number& scale_factor, const Number& aversion_ratio, const Number& min_separation = 0);

 protected:
  void SwapBeads(Necklace::Ptr& necklace) const;
}; // struct ComputeValidPlacementAnyOrder

} // namespace necklace_map
} // namespace geoviz

#endif //GEOVIZ_NECKLACE_MAP_COMPUTE_VALID_PLACEMENT_H
