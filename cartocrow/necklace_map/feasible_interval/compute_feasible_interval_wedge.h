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

#ifndef CARTOCROW_NECKLACE_MAP_COMPUTE_FEASIBLE_WEDGE_INTERVAL_H
#define CARTOCROW_NECKLACE_MAP_COMPUTE_FEASIBLE_WEDGE_INTERVAL_H

#include "../../core/core.h"
#include "../necklace.h"
#include "../parameters.h"
#include "../range.h"
#include "compute_feasible_interval.h"

namespace cartocrow::necklace_map {

/// A functor to generate feasible wedge intervals for necklace bead placement.
///
/// The generated wedge interval is the intersection of the necklace and a wedge
/// \f$W\f$, such that the apex of \f$W\f$ is the necklace kernel, \f$W\f$
/// contains a map region, and the inner angle of \f$W\f$ is minimal.
///
/// If the region contains the necklace kernel, the wedge interval would cover
/// the complete plane. In this case, a centroid interval in generated instead.
class ComputeFeasibleWedgeInterval : public ComputeFeasibleInterval {
  public:
	CircularRange operator()(const PolygonSet<Inexact>& shape, const Necklace& necklace) const;
	ComputeFeasibleWedgeInterval(const Parameters& parameters);

  private:
	friend ComputeFeasibleInterval;

	Number<Inexact> interval_length_min_rad_;

	std::shared_ptr<ComputeFeasibleInterval> fallback_point_regions_;
	std::shared_ptr<ComputeFeasibleInterval> fallback_kernel_region_;
	std::shared_ptr<ComputeFeasibleInterval> fallback_small_regions_;
};

} // namespace cartocrow::necklace_map

#endif //CARTOCROW_NECKLACE_MAP_COMPUTE_FEASIBLE_WEDGE_INTERVAL_H
