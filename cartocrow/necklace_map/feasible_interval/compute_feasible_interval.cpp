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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 05-12-2019
*/

#include "compute_feasible_interval.h"

#include <cmath>

#include "compute_feasible_interval_centroid.h"
#include "compute_feasible_interval_wedge.h"

namespace cartocrow::necklace_map {

std::shared_ptr<ComputeFeasibleInterval>
ComputeFeasibleInterval::construct(const Parameters& parameters) {
	switch (parameters.interval_type) {
	case IntervalType::kCentroid:
		return std::make_shared<ComputeFeasibleCentroidInterval>(parameters);
	case IntervalType::kWedge: {
		// the wedge interval functor also needs a centroid interval functor
		// as fallback
		auto compute_wedge = std::make_shared<ComputeFeasibleWedgeInterval>(parameters);

		ComputeFeasibleWedgeInterval* functor =
		    static_cast<ComputeFeasibleWedgeInterval*>(compute_wedge.get());
		functor->fallback_point_regions_.reset(new ComputeFeasibleCentroidInterval(parameters));
		functor->fallback_kernel_region_.reset(new ComputeFeasibleCentroidInterval(parameters));

		Parameters small_regions_parameters = parameters;
		small_regions_parameters.centroid_interval_length_rad =
		    parameters.wedge_interval_length_min_rad;
		functor->fallback_small_regions_.reset(
		    new ComputeFeasibleCentroidInterval(small_regions_parameters));

		return compute_wedge;
	}
	default:
		return nullptr;
	}
}

void ComputeFeasibleInterval::operator()(std::shared_ptr<Bead>& bead, const Necklace& necklace) const {
	// TODO possibly want to do this approximation beforehand?
	bead->feasible = (*this)(approximate(bead->region->shape), necklace);
}

/*void ComputeFeasibleInterval::operator()(std::vector<MapElement::Ptr>& elements) const {
	for (MapElement::Ptr& element : elements) {
		(*this)(element);
	}
}*/

ComputeFeasibleInterval::ComputeFeasibleInterval(const Parameters& parameters) {}

} // namespace cartocrow::necklace_map
