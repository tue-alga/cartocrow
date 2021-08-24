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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 02-04-2020
*/

#include "compute_scale_factor_any_order.h"

#include <algorithm>
#include <list>
#include <memory>

#include "cartocrow/necklace_map/bead.h"

namespace cartocrow {
namespace necklace_map {
namespace detail {

ComputeScaleFactorAnyOrder::ComputeScaleFactorAnyOrder(const Necklace::Ptr& necklace,
                                                       const Number& buffer_rad /*= 0*/,
                                                       const int binary_search_depth /*= 10*/,
                                                       const int heuristic_cycles /*= 5*/
                                                       )
    : necklace_shape_(necklace->shape), half_buffer_rad_(0.5 * buffer_rad), max_buffer_rad_(0),
      binary_search_depth_(binary_search_depth) {
	// Collect and order the beads based on the start of their valid interval (initialized as their feasible interval).
	for (const Bead::Ptr& bead : necklace->beads) {
		nodes_.push_back(std::make_shared<CycleNodeLayered>(bead));
	}

	std::sort(nodes_.begin(), nodes_.end(), CompareCycleNodeLayered());

	// Prepare the feasibility check.
	check_ = CheckFeasible::New(nodes_, heuristic_cycles);
}

Number ComputeScaleFactorAnyOrder::Optimize() {
	// Assign a layer to each node such that the nodes in a layer do not overlap in their feasibile intervals.
	const int num_layers = AssignLayers();

	// The algorithm is exponential in the number of layers, so we limit this number.
	if (kMaxLayers < num_layers) {
		return 0;
	}

	// Initialize the collection of task slices: collections of fixed tasks that are relevant within some angle range.
	check_->Initialize();

	// Perform a binary search on the scale factor, determining which are feasible.
	// This binary search requires a decent initial upper bound on the scale factor.
	Number lower_bound = 0;
	Number upper_bound = ComputeScaleUpperBound();

	for (int step = 0; step < binary_search_depth_; ++step) {
		const Number scale_factor = 0.5 * (lower_bound + upper_bound);
		ComputeCoveringRadii(scale_factor);

		if ((*check_)()) {
			lower_bound = scale_factor;
		} else {
			upper_bound = scale_factor;
		}
	}

	ComputeBufferUpperBound(lower_bound);

	// The lower bound is the largest confirmed scale factor for which all beads could fit.
	return lower_bound;
}

Number ComputeScaleFactorAnyOrder::ComputeScaleUpperBound() {
	// The initial upper bound makes sure none of the beads would become too large (i.e. contain the kernel).
	Number upper_bound = 0;
	max_buffer_rad_ = 0;
	for (const CycleNodeLayered::Ptr& node : nodes_) {
		const Number covering_radius_rad =
		    necklace_shape_->ComputeCoveringRadiusRad(node->valid, node->bead->radius_base);
		const Number scale_factor = (M_PI_2 - half_buffer_rad_) / covering_radius_rad;
		upper_bound = 0 < upper_bound ? std::min(upper_bound, scale_factor) : scale_factor;

		// The maximum buffer will be based on the minimum radius and the final scale factor.
		if (0 < covering_radius_rad) {
			max_buffer_rad_ = 0 < max_buffer_rad_ ? std::min(max_buffer_rad_, covering_radius_rad)
			                                      : covering_radius_rad;
		}
	}

	// Perform a binary search to find the largest scale factor for which all beads could fit.
	Number lower_bound = 0.0;
	for (int step = 0; step < binary_search_depth_; ++step) {
		const Number scale_factor = 0.5 * (lower_bound + upper_bound);

		Number totalSize = 0.0;
		for (const CycleNodeLayered::Ptr& node : nodes_) {
			totalSize += necklace_shape_->ComputeCoveringRadiusRad(
			                 node->valid, scale_factor * node->bead->radius_base) +
			             half_buffer_rad_;
		}

		// Check whether the scaled beads could fit.
		if (totalSize <= M_PI) {
			lower_bound = scale_factor;
		} else {
			upper_bound = scale_factor;
		}
	}

	// The lower bound is the largest confirmed scale factor for which all beads could fit.
	return lower_bound;
}

void ComputeScaleFactorAnyOrder::ComputeCoveringRadii(const Number& scale_factor) {
	for (CycleNodeLayered::Ptr& node : nodes_) {
		node->bead->covering_radius_rad = necklace_shape_->ComputeCoveringRadiusRad(
		                                      node->valid, node->bead->radius_base * scale_factor) +
		                                  half_buffer_rad_;
	}
}

int ComputeScaleFactorAnyOrder::AssignLayers() {
	// Each node should be assigned a layer such that each layer does not contain any pair of nodes that overlap in their valid interval.
	// Note that this can be done greedily: assign the nodes by minimizing the distance between the last valid interval and the next.

	using NodeList = std::list<CycleNodeLayered::Ptr>;
	NodeList remaining_nodes(nodes_.begin(), nodes_.end());

	int layer = 0;
	remaining_nodes.front()->layer = layer;
	CircularRange layer_interval(*remaining_nodes.front()->valid);

	remaining_nodes.pop_front();
	NodeList::iterator node_iter = remaining_nodes.begin();
	NodeList::iterator unused_iter = remaining_nodes.end();

	// Note that the nodes are already ordered by the starting angle of their valid interval.
	while (!remaining_nodes.empty()) {
		if (!layer_interval.IntersectsOpen((*node_iter)->valid)) {
			// Add the non-overlapping node to the layer.
			(*node_iter)->layer = layer;
			layer_interval.to_rad() =
			    ModuloNonZero((*node_iter)->valid->to(), layer_interval.from_rad());
			node_iter = remaining_nodes.erase(node_iter);
		} else if (node_iter == unused_iter) {
			// All nodes were checked: start a new layer.
			++layer;
			(*node_iter)->layer = layer;
			layer_interval = CircularRange(*(*node_iter)->valid);

			node_iter = remaining_nodes.erase(node_iter);
			unused_iter = remaining_nodes.end();
		} else {
			if (unused_iter == remaining_nodes.end()) {
				// Mark the node as the first one of the next layer.
				unused_iter = node_iter;
			}
			++node_iter;
		}

		if (node_iter == remaining_nodes.end()) {
			node_iter = remaining_nodes.begin();
		}
	}

	return layer + 1;
}

void ComputeScaleFactorAnyOrder::ComputeBufferUpperBound(const Number& scale_factor) {
	max_buffer_rad_ *= scale_factor;
}

} // namespace detail
} // namespace necklace_map
} // namespace cartocrow
