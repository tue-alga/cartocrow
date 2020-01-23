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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 05-12-2019
*/

#ifndef GEOVIZ_NECKLACE_MAP_COMPUTE_FEASIBLE_INTERVAL_H
#define GEOVIZ_NECKLACE_MAP_COMPUTE_FEASIBLE_INTERVAL_H

#include <memory>
#include <unordered_map>
#include <vector>

#include "geoviz/common/core_types.h"
#include "geoviz/necklace_map/bead.h"
#include "geoviz/necklace_map/map_element.h"
#include "geoviz/necklace_map/necklace.h"


namespace geoviz
{
namespace necklace_map
{

struct ComputeFeasibleInterval
{
  virtual CircleRange::Ptr operator()
  (
    const Polygon& extent,
    const Necklace::Ptr& necklace
  ) const = 0;

  void operator()(MapElement::Ptr& element) const;

  void operator()(std::vector<MapElement::Ptr>& elements) const;
}; // struct ComputeFeasibleInterval


struct ComputeFeasibleCentroidInterval : public ComputeFeasibleInterval
{
  ComputeFeasibleCentroidInterval(const Number& length_rad);

  CircleRange::Ptr operator()
  (
    const Polygon& extent,
    const Necklace::Ptr& necklace
  ) const;

 private:
  Number half_length_rad_;
}; // struct ComputeFeasibleCentroidInterval


struct ComputeFeasibleWedgeInterval : public ComputeFeasibleInterval
{
  CircleRange::Ptr operator()
  (
    const Polygon& extent,
    const Necklace::Ptr& necklace
  ) const;
}; // struct ComputeFeasibleWedgeInterval

} // namespace necklace_map
} // namespace geoviz

#endif //GEOVIZ_NECKLACE_MAP_COMPUTE_FEASIBLE_INTERVAL_H
