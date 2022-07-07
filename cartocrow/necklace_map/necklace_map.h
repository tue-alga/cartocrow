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

#include <cartocrow/core/core_types.h>
#include <cartocrow/core/region.h>

#include "compute_feasible_interval.h"
#include "compute_scale_factor.h"
#include "compute_valid_placement.h"
#include "io/data_reader.h"
#include "io/ipe_reader.h"
#include "io/type_parsers.h"
#include "map_element.h"
#include "necklace.h"
#include "parameters.h"

namespace cartocrow {
namespace necklace_map {

Number ComputeScaleFactor(const Parameters& parameters, std::vector<MapElement::Ptr>& elements,
                          std::vector<Necklace::Ptr>& necklaces);

void ComputePlacement(const Parameters& parameters, const Number& scale_factor,
                      std::vector<MapElement::Ptr>& elements, std::vector<Necklace::Ptr>& necklaces);

} // namespace necklace_map

} // namespace cartocrow

#endif //CARTOCROW_NECKLACE_MAP_NECKLACE_MAP_H
