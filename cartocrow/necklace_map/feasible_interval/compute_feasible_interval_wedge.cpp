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

#include "compute_feasible_interval_wedge.h"

#include "../necklace_interval.h"
#include "cartocrow/core/core.h"

namespace cartocrow::necklace_map {

CircularRange ComputeFeasibleWedgeInterval::operator()(const PolygonSet<Inexact>& shape,
                                                       const Necklace& necklace) const {
	// TODO code commented out for the sake of getting this to work
	/*assert(extent.size() > 0);
	if (extent.size() == 1) {
		return (*fallback_point_regions_)(extent, necklace);
	}

	const Number angle = necklace->shape->ComputeAngleRad(*extent.vertices_begin());
	CircularRange obscured(angle, angle);

	const Point& kernel = necklace->shape->kernel();
	for (Polygon::Edge_const_iterator edge_iter = extent.edges_begin();
	     edge_iter != extent.edges_end(); ++edge_iter) {
		const Segment& segment = *edge_iter;
		const Number angle_target = necklace->shape->ComputeAngleRad(segment[1]);

		if (CGAL::left_turn(segment[0], segment[1], kernel)) {
			// Counterclockwise segment.
			obscured.to_rad() =
			    std::max(obscured.to_rad(), ModuloNonZero(angle_target, obscured.from_rad()));
		} else {
			// Clockwise segment.
			Number angle_target_adj = angle_target;
			while (obscured.to_rad() < angle_target_adj) {
				angle_target_adj -= M_2xPI;
			}
			obscured.from_rad() = std::min(obscured.from_rad(), angle_target_adj);
		}
	}

	const Number interval_length = obscured.ComputeLength();
	if (interval_length == 0) {
		return (*fallback_point_regions_)(extent, necklace);
	} else if (M_2xPI <= interval_length) {
		return (*fallback_kernel_region_)(extent, necklace);
	} else if (interval_length < interval_length_min_rad_) {
		return (*fallback_small_regions_)(extent, necklace);
	}

	// Force the angles into the correct interval.
	return std::make_shared<IntervalWedge>(Modulo(obscured.from_rad()),
	                                       ModuloNonZero(obscured.to_rad(), obscured.from_rad()));*/

	// TODO debug
	return CircularRange(0, M_2xPI);
}

ComputeFeasibleWedgeInterval::ComputeFeasibleWedgeInterval(const Parameters& parameters)
    : ComputeFeasibleInterval(parameters),
      interval_length_min_rad_(parameters.wedge_interval_length_min_rad) {}

} // namespace cartocrow::necklace_map
