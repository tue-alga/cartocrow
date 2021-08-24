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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 10-09-2019
*/

#ifndef CARTOCROW_NECKLACE_MAP_NECKLACE_MAP_H
#define CARTOCROW_NECKLACE_MAP_NECKLACE_MAP_H

#include <vector>

#include <cartocrow/common/core_types.h>
#include <cartocrow/common/region.h>

#include "cartocrow/necklace_map/compute_feasible_interval.h"
#include "cartocrow/necklace_map/compute_scale_factor.h"
#include "cartocrow/necklace_map/compute_valid_placement.h"
#include "cartocrow/necklace_map/map_element.h"
#include "cartocrow/necklace_map/necklace.h"
#include "cartocrow/necklace_map/parameters.h"
#include "cartocrow/necklace_map/io/data_reader.h"
#include "cartocrow/necklace_map/io/svg_reader.h"
#include "cartocrow/necklace_map/io/svg_writer.h"
#include "cartocrow/necklace_map/io/type_parsers.h"


namespace cartocrow
{
namespace necklace_map
{

Number ComputeScaleFactor
(
  const Parameters& parameters,
  std::vector<MapElement::Ptr>& elements,
  std::vector<Necklace::Ptr>& necklaces
);

void ComputePlacement
(
  const Parameters& parameters,
  const Number& scale_factor,
  std::vector<MapElement::Ptr>& elements,
  std::vector<Necklace::Ptr>& necklaces
);

} // namespace necklace_map

} // namespace cartocrow

#endif //CARTOCROW_NECKLACE_MAP_NECKLACE_MAP_H
