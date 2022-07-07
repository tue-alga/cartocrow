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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 28-01-2020
*/

#ifndef CARTOCROW_NECKLACE_MAP_PARAMETERS_H
#define CARTOCROW_NECKLACE_MAP_PARAMETERS_H

#include "../core/core.h"

namespace cartocrow {
namespace necklace_map {

/// A type of feasible interval on a necklace.
enum class IntervalType { kCentroid, kWedge };

/// A type of ordering to apply when computing the optimal scale factor and bead placement.
enum class OrderType { kFixed, kAny };

/// A struct to collect the parameters used for computing the necklace map.
/**
 * These parameters include those needed for computing the feasible intervals,
 * the optimal scale factor, and a valid placement for the necklace beads.
 */
struct Parameters {
	/// Construct a collection of parameters.
	/// All parameters are initialized as valid values.
	Parameters();

	/// The type of feasible intervals to compute.
	IntervalType interval_type;
	/// The length of any centroid intervals generated when computing the
	/// feasible intervals.
	Number centroid_interval_length_rad;
	/// The minimum length of any wedge interval generated when computing the
	/// feasible intervals.
	/// If a generated wedge interval is shorter than this length, it is
	/// replaced by a centroid interval with this length.
	Number wedge_interval_length_min_rad;
	/// Whether to ignore degenerate (point) regions.
	/// Non-ignored point regions are always assigned a centroid region.
	bool ignore_point_regions;
	/// The type of order imposed on the necklace beads.
	/// This order is used when computing the optimal scale factor and when
	/// computing a valid placement.
	OrderType order_type;
	/// The minimum angle in radians of the empty wedge between neighboring
	/// necklace beads that has the necklace kernel as apex.
	/// This buffer is used when computing the optimal scale factor and when
	/// computing a valid placement.
	Number buffer_rad;
	/// The depth of the binary search tree used for the any-order decision
	/// problem.
	/// A larger depth will produce higher precision at the cost of processing
	/// time.
	int binary_search_depth;
	/// The number of steps for the heuristic any-order scale factor
	/// computation.
	/// If the number of steps is 0, the exact algorithm is used. Otherwise, a
	/// larger number of steps results in a higher probability of generating the
	/// correct outcome of the any-order scale computation decision problem.
	int heuristic_cycles;
	/// The number of steps for the placement heuristic.
	/// Must be non-negative. If the number of cycles is 0, all beads are placed
	/// in the most clockwise valid position.
	int placement_cycles;
	/// The ratio between attraction to the interval center (0) and repulsion
	/// from the neighboring beads (1).
	/// This ratio must be in the range (0, 1].
	Number aversion_ratio;
}; // struct Parameters

} // namespace necklace_map
} // namespace cartocrow

#endif //CARTOCROW_NECKLACE_MAP_PARAMETERS_H
