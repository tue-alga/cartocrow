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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 06-05-2020
*/

#ifndef CARTOCROW_NECKLACE_MAP_DETAIL_COMPUTE_SCALE_FACTOR_ANY_ORDER_INGOT_H
#define CARTOCROW_NECKLACE_MAP_DETAIL_COMPUTE_SCALE_FACTOR_ANY_ORDER_INGOT_H

#include "compute_scale_factor_any_order.h"

#include "../../core/core.h"
#include "../necklace.h"

namespace cartocrow {
namespace necklace_map {
namespace detail {

// Compute the scale factor where beads can be placed in any order and the beads have ingot shapes.
// All ingots will use the same (maximal) covering interval, and instead represent the data value by their length.
class ComputeScaleFactorAnyOrderIngot : public ComputeScaleFactorAnyOrder {
  public:
	ComputeScaleFactorAnyOrderIngot(const Necklace::Ptr& necklace, const Number& buffer_rad = 0,
	                                const int binary_search_depth = 10,
	                                const int heuristic_cycles = 5);

  protected:
	Number ComputeScaleUpperBound() override;

	void ComputeCoveringRadii(const Number& scale_factor) override;
}; // class ComputeScaleFactorAnyOrderIngot

} // namespace detail
} // namespace necklace_map
} // namespace cartocrow

#endif //CARTOCROW_NECKLACE_MAP_DETAIL_COMPUTE_SCALE_FACTOR_ANY_ORDER_INGOT_H
