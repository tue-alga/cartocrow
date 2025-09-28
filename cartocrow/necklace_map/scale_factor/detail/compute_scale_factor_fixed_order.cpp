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

#include "compute_scale_factor_fixed_order.h"

#include <CGAL/Arr_curve_data_traits_2.h>
#include <CGAL/Arr_linear_traits_2.h>
#include <CGAL/Envelope_diagram_1.h>
#include <CGAL/envelope_2.h>

namespace cartocrow::necklace_map {
namespace detail {

/**@class ComputeScaleFactorFixedOrder
 * @brief Computes the scale factor for collections ordered by their interval.
 */

/**@brief Construct a fixed order scale factor computation functor.
 * @param necklace the necklace for which to compute the scale factor.
 * @param buffer_rad the minimum angle in radians of the empty wedge between neighboring necklace beads that has the necklace kernel as apex.
 */
ComputeScaleFactorFixedOrder::ComputeScaleFactorFixedOrder(Necklace& necklace,
                                                           Number<Inexact> buffer_rad)
    : nodes_(), necklace_shape_(necklace.shape), buffer_rad_(buffer_rad), max_buffer_rad_(-1) {
	// The necklace must be sorted by the feasible intervals of its beads.
	necklace.sortBeads();

	// Add a node to the scale factor computation functor per element.
	nodes_.reserve(2 * necklace.beads.size());
	for (const std::shared_ptr<Bead>& bead : necklace.beads) {
		assert(bead->radius_base > 0);
		// Ignore beads without a feasible interval.

		// Compute the covering radius.
		// This metric will be compared to the feasible intervals when determining how close together beads can be placed.
		// For this reason, the covering radius must be in the same unit as the feasible intervals, so in radians describing the wedges around the necklace kernel.
		// For circle necklaces, this only depends on the radius of the unscaled bead.
		// However, for Bezier necklaces, the distance between the necklace and the kernel is not constant and neither is the local curvature of the necklace.
		// For example, the intersection of a bead with fixed radius and the necklace will fall in a smaller wedge if the bead is placed further from the necklace kernel or if the necklace is oriented less orthogonal to the line connecting kernel and bead.
		// As a safe approximation of a fixed covering radius, the largest covering radius that a bead can have when placed inside its feasible interval is used.
		//
		// Note that when considering Bezier splines that are far from circular, using the intersection of the bead with the necklace actually breaks down: there can be situations where a fixed size bead can be placed in two non-contiguous intervals but not the space between them.
		// We should probably solve this by using non-overlapping wedges as opposed to the part of the necklace covered by the bead...

		bead->covering_radius_rad =
		    necklace.shape->computeCoveringRadiusRad(bead->feasible, bead->radius_base);
		// Note that for an exact computation, the scaling factor should be inside this arcsine function.
		// This can be solved by performing a bisection search on the scale factors using a feasibility check to see if the scaled beads fit.
		// Note that this correction is performed after estimating the scale factor, in CorrectScaleFactor().

		bead->angle_rad = bead->feasible.from();

		nodes_.emplace_back(bead);
	}

	// Each node is duplicated with an offset to its interval to force cyclic validity.
	const NodeSet::iterator end = nodes_.end();
	for (NodeSet::iterator node_iter = nodes_.begin(); node_iter != end; ++node_iter) {
		assert(node_iter->bead != nullptr);
		nodes_.emplace_back(node_iter->bead);

		nodes_.back().valid->from() += M_2xPI;
		nodes_.back().valid->to() += M_2xPI;
	}
}

/**@brief Compute the optimal scale factor.
 * @return the maximum value by which the necklace bead radii can be multiplied such that they maintain the required buffer size.
 */
Number<Inexact> ComputeScaleFactorFixedOrder::Optimize() {
	// Check whether the buffer allows any nodes.
	// Note that the nodes are inserted twice.
	max_buffer_rad_ = M_2xPI / (size() / 2); // TODO possible bug: integer division?
	const Number<Inexact> available_rad = M_2xPI - buffer(0, size() / 2);

	const Number<Inexact> rho = OptimizeSubProblem(0, size() - 1, max_buffer_rad_);
	const Number<Inexact> rho_full_necklace =
	    available_rad <= 0
	        ? 0
	        : available_rad /
	              (2 * r(0, (size() / 2) - 1)); // Note that the necklace beads were added twice.
	return rho < 0 ? rho_full_necklace : std::min(CorrectScaleFactor(rho), rho_full_necklace);
}

/**@fn const Number<Inexact>& ComputeScaleFactorFixedOrder::max_buffer_rad() const;
 * @brief Get the minimum angle in radians of the empty wedge between neighboring necklace beads that has the necklace kernel as apex.
 * @return the buffer size in radians.
 */

inline size_t ComputeScaleFactorFixedOrder::size() const {
	return nodes_.size();
}

// Buffer between i and j.
inline Number<Inexact> ComputeScaleFactorFixedOrder::buffer(const size_t i, const size_t j) const {
	assert(i <= j);
	const ptrdiff_t num_buffers = j - i;
	return num_buffers * buffer_rad_;
}

// Interval start a_i.
inline const Number<Inexact>& ComputeScaleFactorFixedOrder::a(const size_t i) const {
	return nodes_[i].valid->from();
}

// Interval end b_i.
inline const Number<Inexact>& ComputeScaleFactorFixedOrder::b(const size_t i) const {
	return nodes_[i].valid->to();
}

// Radius r_i.
inline const Number<Inexact>& ComputeScaleFactorFixedOrder::r(const size_t i) const {
	return nodes_[i].bead->covering_radius_rad;
}

Number<Inexact> ComputeScaleFactorFixedOrder::r(const size_t i, const size_t j) const {
	// Note that we could store (partial) results, but the gains would be minimal.
	Number<Inexact> aggregate_radius = 0;
	for (size_t n = i; n <= j; ++n) {
		aggregate_radius += r(n);
	}
	return aggregate_radius;
}

Number<Inexact> ComputeScaleFactorFixedOrder::CorrectScaleFactor(const Number<Inexact>& rho) const {
	// Determine a lower bound on the scale factor by reverse engineering based on the dilated covering radius.
	// Note that while this forces the scale factor to be such that none of the scaled beads cover more than their scaled covering radius, the scale factor may often be increased slightly to exploit the freed up space on the scaled covering radius of the bead's neighbors.
	Number<Inexact> scale_factor = rho;
	for (size_t n = 0; n < size(); ++n) {
		// After scaling the angle, we should determine the new bead radius such that it falls inside the wedge and base the scaling on that.
		// Given a angle scale factor rho, covering (angle) radius c, necklace radius R, bead base radius r, and bead radius scale factor rho',
		// rho' = R sin(rho * c) / r  (the scaled bead touches the edge of the scaled wedge)
		//   c = asin(r / R)  =>  R = r / sin(c)
		//   r * rho' = r * sin(rho * c) / sin(c)
		// rho' = sin(rho * c) / sin(c)
		const Number<Inexact> rho_prime = std::sin(rho * r(n)) / std::sin(r(n));
		scale_factor = std::min(scale_factor, rho_prime);
	}
	return scale_factor;
}

// Note that this computes the subproblem including J (unlike C++ customs defining the end as one-past).
Number<Inexact> ComputeScaleFactorFixedOrder::OptimizeSubProblem(const size_t I, const size_t J,
                                                                 Number<Inexact>& max_buffer_rad) const {
	const size_t size = J - I + 1;
	assert(size >= 1);
	switch (size) {
	// Return solutions to minimal problems.
	case 0:
	case 1:
		return -1;
	case 2: {
		const Number<Inexact> interval_length = b(J) - a(I);
		max_buffer_rad = std::min(max_buffer_rad, interval_length);
		if (interval_length <= buffer(I, J)) {
			return 0;
		}
		return (interval_length - buffer(I, J)) /
		       (r(I) + r(J)); // rho_IJ = (b_J - a_I) / (2*r_IJ - r_I - r_J)
	}
	default: {
		// Compute the scale factor using divide-and-conquer:
		// split the problem into two sub-problems with roughly half the size.
		const size_t k = (I + J) / 2;
		const Number<Inexact> rho_1 = OptimizeSubProblem(I, k, max_buffer_rad);
		const Number<Inexact> rho_2 = OptimizeSubProblem(k + 1, J, max_buffer_rad);

		// For the conquer part, we also need the smallest rho_ij where I <= i <= k < j <= J.
		// This smallest rho_ij is the lowest intersection (over all i,j | i <= k < j) of l_i, r_j,
		// where l_i = (X - a_i) / (2*r_ik - r_i) and r_j = (b_j - X) / (2*r_mj - r_j)
		// [so rho_ij = (b_j - a_i) / (2*r_ik - r_i + 2*r_mj - r_j) = (b_j - a_i) / (2*r_ij - r_i - r_j)].
		using Linear_traits = CGAL::Arr_linear_traits_2<Inexact>;
		using Curve_traits = CGAL::Arr_curve_data_traits_2<Linear_traits, size_t>;
		using Monotone_line = Curve_traits::X_monotone_curve_2;
		using Envelope_diagram = CGAL::Envelope_diagram_1<Curve_traits>;

		std::vector<Monotone_line> lines;
		lines.reserve(nodes_.size());
		for (size_t i = I; i <= k; ++i) {
			const Number<Inexact> x = 1 / (2 * r(i, k) - r(i));
			const Number<Inexact> y = (a(i) + buffer(i, k)) * x;
			assert(x >= 0);

			const Line<Inexact> line(x, -1, -y);
			lines.push_back(Monotone_line(line, i));
		}
		for (size_t j = k + 1; j <= J; ++j) {
			const Number<Inexact> x = -1 / (2 * r(k + 1, j) - r(j));
			const Number<Inexact> y = (b(j) - buffer(k, j)) * x;
			assert(x <= 0);

			const Line<Inexact> line(x, -1, -y);
			lines.push_back(Monotone_line(line, j));
		}

		// Compute the lower envelope.
		Envelope_diagram diagram;
		CGAL::lower_envelope_x_monotone_2(lines.begin(), lines.end(), diagram);

		Number<Inexact> rho = -1;
		for (Envelope_diagram::Edge_const_handle edge_iter = diagram.leftmost();
		     edge_iter != diagram.rightmost(); edge_iter = edge_iter->right()->right()) {
			const size_t& i = edge_iter->curve().data();
			const size_t& j = edge_iter->right()->right()->curve().data();
			if (i <= k && k < j) {
				rho = edge_iter->right()->point().y();
				max_buffer_rad = std::min(max_buffer_rad, (b(j) - a(i)) / (j - i));

				if (b(j) - a(i) < buffer(i, j)) {
					return 0;
				}

				// Note that there is always exactly one vertex of the envelope incident to lines on either side of k.
				break;
			}
		}
		assert(rho >= 0);

		//      // Alternatively, we could compute all O(n^2) combinations of i and j.
		//      Number<Inexact> rho__ = -1;
		//      for (size_t i = I; i <= k; ++i)
		//      {
		//        for (size_t j = k + 1; j <= J; ++j)
		//        {
		//          const Number<Inexact> interval_length = b(j) - a(i) - buffer(i, j);
		//          const Number<Inexact> rho_ij = interval_length / (2 * r(i, j) - r(i) - r(j));
		//          if (rho__ == -1 || rho_ij < rho__)
		//            rho__ = rho_ij;
		//        }
		//      }
		//      assert(rho__ >= 0);
		//      CHECK_NEAR(rho, rho__, 0.005);

		// The scaling factor is the minimum or rho_1, rho_2, and rho (ignoring negative values).
		Number<Inexact> scale_factor = rho;
		if (0 <= rho_1 && rho_1 < scale_factor) {
			scale_factor = rho_1;
		}
		if (0 <= rho_2 && rho_2 < scale_factor) {
			scale_factor = rho_2;
		}
		return scale_factor;
	}
	}
}

} // namespace detail
} // namespace cartocrow::necklace_map
