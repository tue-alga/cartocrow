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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 06-01-2020
*/

#ifndef CARTOCROW_NECKLACE_MAP_DETAIL_COMPUTE_SCALE_FACTOR_H
#define CARTOCROW_NECKLACE_MAP_DETAIL_COMPUTE_SCALE_FACTOR_H

#include <memory>

#include "cartocrow/common/core_types.h"
#include "cartocrow/necklace_map/bead.h"
#include "cartocrow/necklace_map/detail/cycle_node.h"
#include "cartocrow/necklace_map/necklace.h"

namespace cartocrow {
namespace necklace_map {
namespace detail {

class ComputeScaleFactorFixedOrder {
  public:
	ComputeScaleFactorFixedOrder(const Necklace::Ptr& necklace, const Number& buffer_rad = 0);

	Number Optimize();

	const Number& max_buffer_rad() const { return max_buffer_rad_; }

  private:
	// Number of nodes.
	size_t size() const;

	// Aggregate buffer between i and j.
	Number buffer(const size_t i, const size_t j) const;

	// Clockwise extreme angle a_i of an interval.
	const Number& a(const size_t i) const;

	// Counterclockwise extreme angle b_i of an interval.
	const Number& b(const size_t i) const;

	// Covering radius r_i.
	const Number& r(const size_t i) const;

	// Aggregate covering radius r_ij.
	Number r(const size_t i, const size_t j) const;

	Number CorrectScaleFactor(const Number& rho) const;

	// Optimize the scale factor for the beads in the range [I, J].
	Number OptimizeSubProblem(const size_t I, const size_t J, Number& max_buffer_rad) const;

  private:
	// Note that the scaler must be able to access the set by index.
	using NodeSet = std::vector<CycleNode>;
	NodeSet nodes_;

	NecklaceShape::Ptr necklace_shape_;
	Number buffer_rad_;
	Number max_buffer_rad_;
}; // class ComputeScaleFactorFixedOrder

} // namespace detail
} // namespace necklace_map
} // namespace cartocrow

#endif //CARTOCROW_NECKLACE_MAP_DETAIL_COMPUTE_SCALE_FACTOR_H
