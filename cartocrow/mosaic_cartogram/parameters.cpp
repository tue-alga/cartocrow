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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 28-01-2020
*/

#include "parameters.h"

namespace cartocrow::necklace_map {

Parameters::Parameters()
    : interval_type(IntervalType::kCentroid), centroid_interval_length_rad(1),
      ignore_point_regions(false), order_type(OrderType::kFixed), buffer_rad(0),
      binary_search_depth(10), heuristic_cycles(5), placement_cycles(30), aversion_ratio(0) {}

} // namespace cartocrow::necklace_map
