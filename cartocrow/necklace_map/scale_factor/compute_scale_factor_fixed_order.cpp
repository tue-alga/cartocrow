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

#include "compute_scale_factor_fixed_order.h"

#include "detail/compute_scale_factor_fixed_order.h"

namespace cartocrow::necklace_map {

ComputeScaleFactorFixedOrder::ComputeScaleFactorFixedOrder(const Parameters& parameters)
    : ComputeScaleFactor(parameters) {}

Number<Inexact> ComputeScaleFactorFixedOrder::operator()(Necklace& necklace) {
	detail::ComputeScaleFactorFixedOrder impl(necklace, buffer_rad_);
	const Number<Inexact> scale_factor = impl.Optimize();

	if (max_buffer_rad_ < 0 || impl.max_buffer_rad() < max_buffer_rad_) {
		max_buffer_rad_ = impl.max_buffer_rad();
	}

	return scale_factor;
}

} // namespace cartocrow::necklace_map
