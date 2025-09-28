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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 28-11-2019
*/

#include "compute_scale_factor.h"

#include "compute_scale_factor_any_order.h"
#include "compute_scale_factor_fixed_order.h"

namespace cartocrow::necklace_map {

std::shared_ptr<ComputeScaleFactor> ComputeScaleFactor::construct(const Parameters& parameters) {
	switch (parameters.order_type) {
	case OrderType::kFixed:
		return std::make_shared<ComputeScaleFactorFixedOrder>(parameters);
	case OrderType::kAny:
		return std::make_shared<ComputeScaleFactorAnyOrder>(parameters);
	default:
		return nullptr;
	}
}

ComputeScaleFactor::ComputeScaleFactor(const Parameters& parameters)
    : buffer_rad_(parameters.buffer_rad), max_buffer_rad_(-1) {
	assert(buffer_rad_ >= 0);
	assert(buffer_rad_ <= M_PI);
}

Number<Inexact> ComputeScaleFactor::operator()(std::vector<Necklace>& necklaces) {
	// determine the optimal scale factor per necklace;
	// the global optimum is the smallest of these
	Number<Inexact> scale_factor = -1;
	for (Necklace& necklace : necklaces) {
		if (necklace.beads.empty()) {
			continue;
		}

		// Limit the initial bead radii.
		Number<Inexact> rescale = 1;
		for (const std::shared_ptr<Bead>& bead : necklace.beads) {
			assert(bead->radius_base > 0);
			const Number<Inexact> distance = necklace.shape->computeDistanceToKernel(bead->feasible);
			const Number<Inexact> bead_rescale = bead->radius_base / distance;
			rescale = std::max(rescale, bead_rescale);
		}
		for (const std::shared_ptr<Bead>& bead : necklace.beads) {
			bead->radius_base /= rescale;
		}

		const Number<Inexact> necklace_scale_factor = (*this)(necklace) / rescale;

		for (const std::shared_ptr<Bead>& bead : necklace.beads) {
			bead->radius_base *= rescale;
		}

		if (scale_factor < 0 || necklace_scale_factor < scale_factor) {
			scale_factor = necklace_scale_factor;
		}
	}
	return std::max(scale_factor, Number<Inexact>(0));
}

} // namespace cartocrow::necklace_map
