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

#ifndef GEOVIZ_NECKLACE_MAP_PARAMETERS_H
#define GEOVIZ_NECKLACE_MAP_PARAMETERS_H

#include "geoviz/common/core_types.h"


namespace geoviz
{
namespace necklace_map
{

enum class IntervalType
{
  kCentroid,
  kWedge
};

enum class OrderType
{
  kFixed,
  kAny
};


struct Parameters
{
  Parameters();

  // Feasible interval.
  IntervalType interval_type;
  Number centroid_interval_length_rad;
  Number wedge_interval_length_min_rad;
  bool ignore_point_regions;

  // Scale factor.
  OrderType order_type;
  Number buffer_rad;

  int binary_search_depth;
  int heuristic_cycles;

  // Placement.
  int placement_cycles;
  Number aversion_ratio;
}; // struct Parameters

} // namespace necklace_map
} // namespace geoviz

#endif //GEOVIZ_NECKLACE_MAP_PARAMETERS_H
