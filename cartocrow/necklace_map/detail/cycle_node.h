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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 23-01-2020
*/

#ifndef CARTOCROW_NECKLACE_MAP_DETAIL_CYCLE_NODE_H
#define CARTOCROW_NECKLACE_MAP_DETAIL_CYCLE_NODE_H

#include <memory>

#include "../../core/core.h"
#include "../bead.h"

namespace cartocrow::necklace_map {
namespace detail {

// A bead with valid interval.
// The main use is in a sorted cycle of beads that may go around the circle multiple times.
struct CycleNode {
	using Ptr = std::shared_ptr<CycleNode>;

	CycleNode(const CycleNode& node);

	explicit CycleNode(const std::shared_ptr<Bead>& bead);

	CycleNode(const std::shared_ptr<Bead>& bead, const std::shared_ptr<Range>& valid);

	std::shared_ptr<Bead> bead;

	// Note that unlike the bead's feasible interval, the valid interval may go outside the [0, 2pi) range.
	std::shared_ptr<Range> valid;

  protected:
	CycleNode();
};

} // namespace detail
} // namespace cartocrow::necklace_map

#endif //CARTOCROW_NECKLACE_MAP_DETAIL_CYCLE_NODE_H
