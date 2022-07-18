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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 02-04-2020
*/

#ifndef CARTOCROW_NECKLACE_MAP_DETAIL_COMPUTE_SCALE_FACTOR_ANY_ORDER_H
#define CARTOCROW_NECKLACE_MAP_DETAIL_COMPUTE_SCALE_FACTOR_ANY_ORDER_H

#include <vector>

#include "../../../core/core.h"
#include "../../check_feasible/check_feasible.h"
#include "../../detail/cycle_node_layered.h"
#include "../../necklace.h"
#include "../../necklace_shape.h"

namespace cartocrow::necklace_map {
namespace detail {

class ComputeScaleFactorAnyOrder {
  protected:
	// Note that the scaler must be able to access the set by index.
	using NodeSet = std::vector<CycleNodeLayered::Ptr>;

  public:
	constexpr static const int kMaxLayers = 15;

	ComputeScaleFactorAnyOrder(const Necklace& necklace, Number<Inexact> buffer_rad = 0,
	                           const int binary_search_depth = 10, const int heuristic_cycles = 5);

	Number<Inexact> Optimize();

  protected:
	virtual Number<Inexact> ComputeScaleUpperBound();

	virtual void ComputeCoveringRadii(const Number<Inexact>& scale_factor);

  private:
	int AssignLayers();

	void ComputeBufferUpperBound(const Number<Inexact>& scale_factor);

  protected:
	NecklaceShape::Ptr necklace_shape_;

	NodeSet nodes_;

	Number<Inexact> half_buffer_rad_;
	Number<Inexact> max_buffer_rad_;

	int binary_search_depth_;
	CheckFeasible::Ptr check_;
}; // class ComputeScaleFactorAnyOrder

} // namespace detail
} // namespace cartocrow::necklace_map

#endif //CARTOCROW_NECKLACE_MAP_DETAIL_COMPUTE_SCALE_FACTOR_ANY_ORDER_H
