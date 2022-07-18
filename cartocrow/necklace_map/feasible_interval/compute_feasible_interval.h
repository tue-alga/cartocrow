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

#ifndef CARTOCROW_NECKLACE_MAP_COMPUTE_FEASIBLE_INTERVAL_H
#define CARTOCROW_NECKLACE_MAP_COMPUTE_FEASIBLE_INTERVAL_H

#include <memory>
#include <set>
#include <vector>

#include "../../core/core.h"
#include "../bead.h"
#include "../circular_range.h"
#include "../necklace.h"
#include "../parameters.h"

namespace cartocrow::necklace_map {

/// An interface for a functor to generate feasible intervals for necklace bead
/// placement.
class ComputeFeasibleInterval {
  public:
	virtual ~ComputeFeasibleInterval() = default;

	/// Constructs a new feasible interval computation functor.
	static std::shared_ptr<ComputeFeasibleInterval> construct(const Parameters& parameters);

	/// Applies the functor to a shape on the given necklace.
	virtual CircularRange operator()(const PolygonSet<Inexact>& extent,
	                                 const Necklace& necklace) const = 0;

	/// Applies the functor to a map element.
	void operator()(std::shared_ptr<Bead>& bead) const;

	/// Applies the functor to a collection of map elements.
	// TODO
	// void operator()(std::vector<NecklaceMapElement::Ptr>& elements) const;

  protected:
	ComputeFeasibleInterval(const Parameters& parameters);
};

} // namespace cartocrow::necklace_map

#endif //CARTOCROW_NECKLACE_MAP_COMPUTE_FEASIBLE_INTERVAL_H
