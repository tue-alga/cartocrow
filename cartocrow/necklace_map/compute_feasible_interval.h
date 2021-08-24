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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 05-12-2019
*/

#ifndef CARTOCROW_NECKLACE_MAP_COMPUTE_FEASIBLE_INTERVAL_H
#define CARTOCROW_NECKLACE_MAP_COMPUTE_FEASIBLE_INTERVAL_H

#include <memory>
#include <set>
#include <vector>

#include "cartocrow/common/circular_range.h"
#include "cartocrow/common/core_types.h"
#include "cartocrow/necklace_map/map_element.h"
#include "cartocrow/necklace_map/necklace.h"
#include "cartocrow/necklace_map/parameters.h"


namespace cartocrow
{
namespace necklace_map
{

class ComputeFeasibleInterval
{
 public:
  using Ptr = std::unique_ptr<ComputeFeasibleInterval>;

  static Ptr New(const Parameters& parameters);

  virtual CircularRange::Ptr operator()(const Polygon& extent, const Necklace::Ptr& necklace) const = 0;

  void operator()(MapElement::Ptr& element) const;

  void operator()(std::vector<MapElement::Ptr>& elements) const;

 protected:
  ComputeFeasibleInterval(const Parameters& parameters);

  std::set<Necklace::Ptr> to_clean_;
}; // class ComputeFeasibleInterval

} // namespace necklace_map
} // namespace cartocrow

#endif //CARTOCROW_NECKLACE_MAP_COMPUTE_FEASIBLE_INTERVAL_H
