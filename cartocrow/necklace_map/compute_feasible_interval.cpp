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

#include <glog/logging.h>

#include "bead.h"
#include "compute_feasible_interval_centroid.h"
#include "compute_feasible_interval_wedge.h"

namespace cartocrow {
namespace necklace_map {

/**@class ComputeFeasibleInterval
 * @brief An interface for a functor to generate feasible intervals for necklace bead placement.
 */

/**@fn ComputeFeasibleInterval::Ptr
 * @brief The preferred pointer type for storing or sharing a computation functor.
 */

/**@brief Construct a new feasible interval computation functor.
 * @param parameters the parameters describing the desired type of functor.
 * @return a unique pointer containing a new functor or a nullptr if the functor could not be constructed.
 */
ComputeFeasibleInterval::Ptr ComputeFeasibleInterval::New(const Parameters& parameters) {
	// The wedge interval functor also needs a centroid interval functor as fallback.
	switch (parameters.interval_type) {
	case IntervalType::kCentroid:
		return Ptr(new ComputeFeasibleCentroidInterval(parameters));
	case IntervalType::kWedge: {
		Ptr compute_wedge(new ComputeFeasibleWedgeInterval(parameters));

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

/**@fn virtual CircularRange::Ptr ComputeFeasibleInterval::operator()(const Polygon& extent, const Necklace::Ptr& necklace) const = 0;
 * @brief Apply the functor to a region and necklace.
 * @param extent the spatial extent of the region.
 * @param necklace the necklace.
 * @return the feasible interval for placing the region's bead on the necklace.
 */

/**@brief Apply the functor to a map element.
 * @param[in,out] element the element.
 */
void ComputeFeasibleInterval::operator()(MapElement::Ptr& element) const {
	Polygon extent;
	element->region.MakeSimple(extent);

	if (!element->bead) {
		return;
	}

	CHECK_NOTNULL(element->necklace);
	element->bead->feasible = (*this)(extent, element->necklace);
}

/**@brief Apply the functor to a collection of map elements.
 * @param[in,out] elements the elements.
 */
void ComputeFeasibleInterval::operator()(std::vector<MapElement::Ptr>& elements) const {
	for (MapElement::Ptr& element : elements) {
		(*this)(element);
	}
}

ComputeFeasibleInterval::ComputeFeasibleInterval(const Parameters& parameters) {}

} // namespace necklace_map
} // namespace cartocrow
