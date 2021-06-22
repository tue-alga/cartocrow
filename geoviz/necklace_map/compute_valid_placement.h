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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 07-01-2020
*/

#ifndef GEOVIZ_NECKLACE_MAP_COMPUTE_VALID_PLACEMENT_H
#define GEOVIZ_NECKLACE_MAP_COMPUTE_VALID_PLACEMENT_H

#include <vector>

#include "geoviz/common/core_types.h"
#include "geoviz/necklace_map/necklace.h"
#include "geoviz/necklace_map/parameters.h"
#include "geoviz/necklace_map/detail/validate_scale_factor.h"


namespace geoviz
{
namespace necklace_map
{

class ComputeValidPlacement
{
 public:
  using Ptr = std::unique_ptr<ComputeValidPlacement>;

  static Ptr New(const Parameters& parameters);

  ComputeValidPlacement(const int cycles, const Number& aversion_ratio, const Number& buffer_rad = 0);

  void operator()(const Number& scale_factor, Necklace::Ptr& necklace) const;

  void operator()(const Number& scale_factor, std::vector<Necklace::Ptr>& necklaces) const;

  int cycles;

  Number aversion_ratio;

  Number buffer_rad;

 protected:
  virtual void SwapBeads(Necklace::Ptr& necklace) const = 0;
}; // class ComputeValidPlacement


class ComputeValidPlacementFixedOrder : public ComputeValidPlacement
{
 public:
  ComputeValidPlacementFixedOrder(const int cycles, const Number& aversion_ratio, const Number& min_separation = 0);

 protected:
  void SwapBeads(Necklace::Ptr& necklace) const override {}
}; // class ComputeValidPlacementFixedOrder


class ComputeValidPlacementAnyOrder : public ComputeValidPlacement
{
 public:
  ComputeValidPlacementAnyOrder(const int cycles, const Number& aversion_ratio, const Number& min_separation = 0);

 protected:
  void SwapBeads(Necklace::Ptr& necklace) const override;
}; // class ComputeValidPlacementAnyOrder

} // namespace necklace_map
} // namespace geoviz

#endif //GEOVIZ_NECKLACE_MAP_COMPUTE_VALID_PLACEMENT_H
