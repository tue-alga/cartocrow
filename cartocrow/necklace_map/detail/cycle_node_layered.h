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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 06-05-2020
*/

#ifndef CARTOCROW_NECKLACE_MAP_DETAIL_CYCLE_NODE_LAYERED_H
#define CARTOCROW_NECKLACE_MAP_DETAIL_CYCLE_NODE_LAYERED_H

#include "cycle_node.h"

namespace cartocrow::necklace_map {
namespace detail {

// A cycle node that can be assigned a layer.
struct CycleNodeLayered : public CycleNode {
	using Ptr = std::shared_ptr<CycleNodeLayered>;

	CycleNodeLayered();

	explicit CycleNodeLayered(const std::shared_ptr<Bead>& bead);

	CycleNodeLayered(const CycleNodeLayered::Ptr& node);

	int layer;
	bool disabled;
}; // struct CycleNodeLayered

class CompareCycleNodeLayered {
  public:
	bool operator()(const CycleNodeLayered::Ptr& a, const CycleNodeLayered::Ptr& b) const;
}; // class CompareCycleNodeLayered

} // namespace detail
} // namespace cartocrow::necklace_map

#endif //CARTOCROW_NECKLACE_MAP_DETAIL_CYCLE_NODE_LAYERED_H
