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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 10-09-2019
*/

#ifndef GEOVIZ_NECKLACE_MAP_NECKLACE_MAP_H
#define GEOVIZ_NECKLACE_MAP_NECKLACE_MAP_H

#include <string>
#include <tuple>
#include <vector>

#include "geoviz/common/core_types.h"
#include "geoviz/common/region.h"
#include "geoviz/necklace_map/compute_feasible_interval.h"
#include "geoviz/necklace_map/compute_scale_factor.h"
#include "geoviz/necklace_map/compute_valid_placement.h"
#include "geoviz/necklace_map/map_element.h"
#include "geoviz/necklace_map/necklace.h"


// TODO(tvl) this should probably end up as forwarding file, to functional parts that may each have their own subdirectory... However, it should probably also contain functionality for running 'the full pipeline'.

// TODO(tvl) make "settings.h" file with parameter struct containing alg type, min separation, aversion factor, etc. Add factory methods to base functors to construct functors based on this struct.

// TODO(tvl) should this file contain the factories?


namespace geoviz
{
namespace necklace_map
{





} // namespace necklace_map

} // namespace geoviz

#endif //GEOVIZ_NECKLACE_MAP_NECKLACE_MAP_H
