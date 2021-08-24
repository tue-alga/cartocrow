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

#include "cartocrow/common/core_types.h"
#include "cartocrow/common/range.h"
#include "cartocrow/necklace_map/compute_feasible_interval.h"
#include "cartocrow/necklace_map/necklace.h"
#include "cartocrow/necklace_map/parameters.h"

namespace cartocrow {
namespace necklace_map {

class ComputeFeasibleWedgeInterval : public ComputeFeasibleInterval {
  public:
	CircularRange::Ptr operator()(const Polygon& extent, const Necklace::Ptr& necklace) const override;

  protected:
	ComputeFeasibleWedgeInterval(const Parameters& parameters);

  private:
	friend ComputeFeasibleInterval;

	Number interval_length_min_rad_;

	ComputeFeasibleInterval::Ptr fallback_point_regions_;
	ComputeFeasibleInterval::Ptr fallback_kernel_region_;
	ComputeFeasibleInterval::Ptr fallback_small_regions_;
}; // class ComputeFeasibleWedgeInterval

} // namespace necklace_map
} // namespace cartocrow

#endif //CARTOCROW_NECKLACE_MAP_COMPUTE_FEASIBLE_WEDGE_INTERVAL_H
