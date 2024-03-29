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

#ifndef CARTOCROW_NECKLACE_MAP_DETAIL_CHECK_FEASIBLE_H
#define CARTOCROW_NECKLACE_MAP_DETAIL_CHECK_FEASIBLE_H

#include <memory>
#include <vector>

#include "../../core/core.h"
#include "../bit_string.h"
#include "../detail/cycle_node_layered.h"
#include "../detail/task.h"

namespace cartocrow::necklace_map {
namespace detail {

// Solve the decision problem defined in the node set: "Is there a valid placement for this set of nodes?"
// Note that the covering radii of the nodes must be pre-computed outside this functor.
class CheckFeasible {
  public:
	virtual ~CheckFeasible() = default;

	using NodeSet = std::vector<CycleNodeLayered::Ptr>;
	using Ptr = std::shared_ptr<CheckFeasible>;

	static Ptr construct(NodeSet& nodes, const int heuristic_cycles);

	void Initialize();

	// Note that the covering radius of each node should be set before calling this.
	virtual bool operator()() = 0;

  protected:
	struct Value {
		Value();

		void Reset();

		Number<Inexact> CoveringRadius() const;

		CycleNodeLayered::Ptr task;
		Number<Inexact> angle_rad; // The angle of the bead center.
	};

	CheckFeasible(NodeSet& nodes);

	virtual void InitializeSlices();

	void InitializeContainer();

	void ResetContainer();

	void FillContainer(const size_t first_slice_index, const BitString& first_slice_layer_set,
	                   const BitString& first_slice_remaining_set);

	virtual void AssignAngle(const Number<Inexact>& angle_rad, std::shared_ptr<Bead>& bead) = 0;

	bool ProcessContainer(const size_t first_slice_index, const BitString& first_slice_remaining_set);

	NodeSet& nodes_;
	std::vector<TaskSlice> slices_;
	std::vector<std::vector<Value>> values_;
};

} // namespace detail
} // namespace cartocrow::necklace_map

#endif //CARTOCROW_NECKLACE_MAP_DETAIL_CHECK_FEASIBLE_H
