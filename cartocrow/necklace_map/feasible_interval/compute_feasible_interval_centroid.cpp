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

#include "compute_feasible_interval_centroid.h"

#include "../../core/centroid.h"
#include "../necklace_interval.h"

namespace cartocrow::necklace_map {

/**@class ComputeFeasibleCentroidInterval
 * @brief A functor to generate feasible centroid intervals for necklace bead placement.
 *
 * The generated centroid interval is the intersection of the necklace and a wedge @f$W@f$, such that the apex of @f$W@f$ is the necklace kernel, the inner bisector of @f$W@f$ intersects the centroid of a map region, and the inner angle of @f$W@f$ is twice some predefined angle.
 *
 * If the centroid of the region is the necklace kernel, the wedge bisector is undefined. In this case the wedge is chosen such that the inner bisector has the same direction as the positive x axis.
 */

CircularRange ComputeFeasibleCentroidInterval::operator()(const PolygonSet<Inexact>& shape,
                                                          const Necklace& necklace) const {
	const Point<Inexact> c = cartocrow::centroid(shape);
	const Number<Inexact> angle_rad = necklace.shape->computeAngleRad(c);

	return IntervalCentroid(angle_rad - half_length_rad_, angle_rad + half_length_rad_);
}

/**@brief Construct a centroid interval generator.
 * @param length_rad @parblock the inner angle (in radians) of the wedge used when generating an interval.
 *
 * The centroid intervals cannot be empty or cover the whole necklace, i.e. the length is restricted to the range (0, 2*pi).
 * @endparblock
 */
ComputeFeasibleCentroidInterval::ComputeFeasibleCentroidInterval(const Parameters& parameters)
    : ComputeFeasibleInterval(parameters),
      half_length_rad_(0.5 * parameters.centroid_interval_length_rad) {
	assert(half_length_rad_ >= 0);
	assert(half_length_rad_ < M_PI);
}

} // namespace cartocrow::necklace_map
