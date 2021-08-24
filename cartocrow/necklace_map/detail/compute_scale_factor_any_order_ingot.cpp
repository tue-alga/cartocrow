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

#include "compute_scale_factor_any_order_ingot.h"

namespace cartocrow {
namespace necklace_map {
namespace detail {

ComputeScaleFactorAnyOrderIngot::ComputeScaleFactorAnyOrderIngot(
    const Necklace::Ptr& necklace, const Number& buffer_rad /*= 0*/,
    const int binary_search_depth /*= 10*/, const int heuristic_cycles /*= 5*/
    )
    : ComputeScaleFactorAnyOrder(necklace, buffer_rad, binary_search_depth, heuristic_cycles) {}

Number ComputeScaleFactorAnyOrderIngot::ComputeScaleUpperBound() {
	max_buffer_rad_ = 0;
	for (const CycleNodeLayered::Ptr& node : nodes_) {
		const Number radius_rad =
		    necklace_shape_->ComputeCoveringRadiusRad(node->valid, node->bead->radius_base);

		// The maximum buffer will be based on the minimum radius and the final scale factor.
		if (0 < radius_rad) {
			max_buffer_rad_ = std::min(max_buffer_rad_, radius_rad);
		}
	}

	return M_PI / nodes_.size() - half_buffer_rad_;
}

void ComputeScaleFactorAnyOrderIngot::ComputeCoveringRadii(const Number& scale_factor) {
	for (CycleNodeLayered::Ptr& node : nodes_) {
		node->bead->covering_radius_rad = scale_factor + half_buffer_rad_;
	}
}

} // namespace detail
} // namespace necklace_map
} // namespace cartocrow
