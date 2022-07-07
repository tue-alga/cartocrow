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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 03-03-2020
*/

#include "compute_scale_factor_any_order.h"

#include "detail/compute_scale_factor_any_order.h"

namespace cartocrow {
namespace necklace_map {

/**@struct ComputeScaleFactorAnyOrder
 * @brief A functor to compute the optimal scale factor for a collection of necklace map elements with undefined order.
 *
 * The optimal scale factor is the maximum value such that if all necklace beads have radius scale factor * sqrt(data value), none of these beads are within the minimum separation distance of another bead on the same necklace.
 *
 * Note that this scale factor is the minimum over the scale factors per necklace. These scale factors per necklace can be determined independently.
 *
 * Note that we do not restrict the beads of different necklaces to overlap. In case of overlap between different necklaces, the user can manually adjust the buffer thickness or the positioning forces (see ComputeValidPlacement) to prevent overlapping beads.
 */

/**@brief Construct a bead scale factor computation functor that is allowed to the order of the beads.
 * @param parameters @parblock the parameter settings to apply to the computations.
 *
 * Specifically parameters.buffer_rad is used to set the minimum distance in radians between necklace beads.
 *
 * This distance must be in the range [0, pi]. Note that values beyond some threshold based on the input regions, the scale factor is forced to 0.
 * @endparblock
 */
ComputeScaleFactorAnyOrder::ComputeScaleFactorAnyOrder(const Parameters& parameters)
    : ComputeScaleFactor(parameters), binary_search_depth_(parameters.binary_search_depth),
      heuristic_cycles_(parameters.heuristic_cycles) {}

Number ComputeScaleFactorAnyOrder::operator()(Necklace::Ptr& necklace) {
	detail::ComputeScaleFactorAnyOrder opt(necklace, buffer_rad_, binary_search_depth_,
	                                       heuristic_cycles_);
	const Number scale_factor = opt.Optimize();

	return scale_factor;
}

} // namespace necklace_map
} // namespace cartocrow
