/*
The Necklace Map library implements the algorithmic geo-visualization
method by the same name, developed by Bettina Speckmann and Kevin Verbeek
at TU Eindhoven (DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
Copyright (C) 2019  Netherlands eScience Center and TU Eindhoven

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

#include "compute_scale_factor.h"

#include <glog/logging.h>


namespace geoviz
{
namespace necklace_map
{
namespace detail
{

/**@class ComputeScaleFactor
 * @brief The base class for computing the scale factor.
 *
 * This mainly implements simple functions that are reused by the specific scale factor computation functors.
 */

/**@brief Construct a scale factor computation functor.
 * @param necklace the necklace for which to compute the scale factor.
 * @param buffer_rad the minimum angle in radians of the empty wedge between neighboring necklace beads that has the necklace kernel as apex.
 */
ComputeScaleFactor::ComputeScaleFactor(const Necklace::Ptr& necklace, const Number& buffer_rad /*= 0*/)
  : max_buffer_rad_(-1), nodes_(), necklace_radius_(necklace->shape->ComputeRadius()), buffer_rad_(buffer_rad)
{
  // Clean beads without a feasible interval.
  std::vector<Bead::Ptr> keep_beads;
  for (const Bead::Ptr& bead : necklace->beads)
    if (bead->feasible)
      keep_beads.push_back(bead);
  necklace->beads.swap(keep_beads);

  // The necklace must be sorted by the feasible intervals of its beads.
  necklace->SortBeads();

  // Per element that should not be ignored (i.e. that has a bead with a feasible interval), add a node to the scale factor computation functor.
  nodes_.reserve(2 * necklace->beads.size());
  for (const Bead::Ptr& bead : necklace->beads)
  {
    CHECK_GT(bead->radius_base, 0);
    // Ignore beads without a feasible interval.

    bead->covering_radius_scaled_rad = std::asin(bead->radius_base / necklace_radius_);
    // Note that for an exact computation, the scaling factor should be inside this arcsine function.
    // This can be solved by performing a bisection search on the scale factors using a feasibility check to see if the scaled beads fit.
    // Note that this correction is performed after estimating the scale factor, in CorrectScaleFactor().

    nodes_.emplace_back(bead);
  }

  // Each node is duplicated with an offset to its interval to force cyclic validity.
  const NodeSet::iterator end = nodes_.end();
  for (NodeSet::iterator node_iter = nodes_.begin(); node_iter != end; ++node_iter)
  {
    CHECK_NOTNULL(node_iter->bead);
    nodes_.emplace_back(node_iter->bead);

    nodes_.back().interval_cw_rad += M_2xPI;
    nodes_.back().interval_ccw_rad += M_2xPI;
  }
}

/**@fn virtual Number ComputeScaleFactor::Optimize() = 0;
 * @brief Compute the optimal scale factor.
 * @return the maximum value by which the necklace bead radii can be multiplied such that they maintain the required buffer size.
 */

/**@fn const Number& ComputeScaleFactor::max_buffer_rad() const;
 * @brief Get the minimum angle in radians of the empty wedge between neighboring necklace beads that has the necklace kernel as apex.
 * @return the buffer size in radians.
 */

inline size_t ComputeScaleFactor::size() const
{
  return nodes_.size();
}

// Buffer between i and j.
inline Number ComputeScaleFactor::buffer(const size_t i, const size_t j) const
{
  CHECK_LE(i, j);
  const ptrdiff_t num_buffers = j - i;
  return num_buffers * buffer_rad_;
}

// Interval start a_i.
inline const Number& ComputeScaleFactor::a(const size_t i) const
{
  return nodes_[i].interval_cw_rad;
}

// Interval end b_i.
inline const Number& ComputeScaleFactor::b(const size_t i) const
{
  return nodes_[i].interval_ccw_rad;
}

// Radius r_i.
inline const Number& ComputeScaleFactor::r(const size_t i) const
{
  return nodes_[i].bead->covering_radius_scaled_rad;
}

Number ComputeScaleFactor::r(const size_t i, const size_t j) const
{
  // Note that we could store (partial) results, but the gains would be minimal.
  Number aggregate_radius = 0;
  for (size_t n = i; n <= j; ++n)
    aggregate_radius += r(n);
  return aggregate_radius;
}

inline Point ComputeScaleFactor::l_(const size_t i, const size_t k) const
{
  Number x = 1 / (2 * r(i, k) - r(i));
  CHECK_GE(x, 0);
  return Point(x, (a(i) + buffer(i, k)) * x);
}

inline Point ComputeScaleFactor::r_(const size_t j, const size_t k) const
{
  Number x = -1 / (2 * r(k + 1, j) - r(j));
  CHECK_LE(x, 0);
  return Point(x, (b(j) - buffer(k, j)) * x);
}

Number ComputeScaleFactor::CorrectScaleFactor(const Number& rho) const
{
  // Determine a lower bound on the scale factor by reverse engineering based on the dilated covering radius.
  // Note that while this forces the scale factor to be such that none of the scaled beads cover more than their scaled covering radius, the scale factor may often be increased slightly to exploit the freed up space on the scaled covering radius of the bead's neighbors.
  Number scale_factor = rho;
  for (size_t n = 0; n < size(); ++n)
  {
    const Number rho_prime =
      necklace_radius_ * std::sin(rho * r(n)) / nodes_[n].bead->radius_base;
    if (rho_prime < scale_factor)
      scale_factor = rho_prime;
  }
  return scale_factor;
}


/**@class ComputeScaleFactorFixedOrder
 * @brief Computes the scale factor for collections ordered by their interval.
 */

/**@brief Construct a fuixed order scale factor computation functor.
 * @param necklace the necklace for which to compute the scale factor.
 * @param buffer_rad the minimum angle in radians of the empty wedge between neighboring necklace beads that has the necklace kernel as apex.
 */
ComputeScaleFactorFixedOrder::ComputeScaleFactorFixedOrder(const Necklace::Ptr& necklace, const Number& buffer_rad /*= 0*/) :
  ComputeScaleFactor(necklace, buffer_rad) {}

Number ComputeScaleFactorFixedOrder::Optimize()
{
  // Check whether the buffer allows any nodes.
  // Note that the nodes are insterted twice.
  max_buffer_rad_ = M_2xPI / (size() / 2);
  const Number total_buffer_rad = buffer(0, size() / 2);

  const Number rho = OptimizeSubProblem(0, size() - 1, max_buffer_rad_);
  const Number rho_fill_circle =
    M_2xPI < total_buffer_rad
    ? 0
    : (M_2xPI - total_buffer_rad) / (2 * r(0, (size() / 2) - 1));  // Note that the necklace beads were added twice.
  return
    rho < 0
    ? rho_fill_circle
    : std::min(CorrectScaleFactor(rho), rho_fill_circle);
  //TODO(tvl) add bisection search for true (valid) scale factor.
}

// Note that this computes the subproblem including J (unlike C++ customs defining the end as one-past).
Number ComputeScaleFactorFixedOrder::OptimizeSubProblem(const size_t I, const size_t J, Number& max_buffer_rad) const
{
  const size_t size = J - I + 1;
  CHECK_GE(size, 1);
  switch (size)
  {
    // Return solutions to minimal problems.
    case 0:
    case 1:
      return -1;
    case 2:
    {
      const Number interval_length = b(J) - a(I);
      max_buffer_rad = std::min(max_buffer_rad, interval_length);
      if (interval_length <= buffer(I, J))
        return 0;
      return (interval_length - buffer(I, J)) / (r(I) + r(J));  // rho_IJ = (b_J - a_I) / (2*r_IJ - r_I - r_J)
    }
    default:
    {
      // Compute the scale factor using divide-and-conquer:
      // split the problem into two sub-problems with roughly half the size.
      const size_t k = (I + J) / 2;
      const Number rho_1 = OptimizeSubProblem(I, k, max_buffer_rad);
      const Number rho_2 = OptimizeSubProblem(k+1, J, max_buffer_rad);

      // For the conquer part, we also need the smallest rho_ij where I <= i <= k < j <= J.
      // This smallest rho_ij is the lowest intersection (over all i,j | i <= k < j) of l_i, r_j,
      // where l_i = (X - a_i) / (2*r_ik - r_i) and r_j = (b_j - X) / (2*r_mj - r_j)
      // [so rho_ij = (b_j - a_i) / (2*r_ik - r_i + 2*r_mj - r_j)] = (b_j - a_i) / (2*r_ij - r_i - r_j).
      // This lowest intersection is the the line of the upper envelope of {L' union R'} that intersects the y axis,
      // where L* is the set of points l*_i = <1 / (2*r_ik - r_i), a_i / (2*r_ik - r_i)>,
      // and R* is the set of points r*_i = <-1 / (2*r_mj - r_j), -b_j / (2*r_mj - r_j)>.

      // Determine the line on the upper envelope that intersects the y axis.
      size_t ii = I, jj = k+1;
      Point l_star = l_(ii, k);
      Point r_star = r_(jj, k);

      for (size_t i = ii+1; i <= k; ++i)
      {
        const Point n_star = l_(i, k);
        if (CGAL::left_turn(r_star, l_star, n_star))
        {
          l_star = n_star;
          ii = i;
        }
      }

      for (size_t j = jj+1; j <= J; ++j)
      {
        const Point n_star = r_(j, k);
        if (CGAL::left_turn(r_star, l_star, n_star))
        {
          r_star = n_star;
          jj = j;
        }
      }

      const Number interval_length = b(jj) - a(ii);

      const Number length_per_bead = interval_length / (jj - ii);
      max_buffer_rad = std::min(max_buffer_rad, length_per_bead);

      if (interval_length <= buffer(ii, jj))
        return 0;

      const Number rho = (interval_length - buffer(ii, jj)) / (2 * r(ii, jj) - r(ii) - r(jj));
      CHECK_GE(rho, 0);

      // Alternatively, we could compute all O(n^2) combinations of i and j.
//      Number rho = -1;
//      for (size_t i = I; i <= k; ++i)
//      {
//        for (size_t j = m; j <= J; ++j)
//        {
//          const Number interval_length = b(j) - a(i) - buffer(i, j);
//          if (interval_length <= 0)
//            return 0;
//
//          const Number rho_ij = interval_length / (2 * r(i, j) - r(i) - r(j));
//          if (rho == -1 || rho_ij < rho)
//            rho = rho_ij;
//          CHECK_GE(rho, 0);
//        }
//      }

      // The scaling factor is the minimum or rho_1, rho_2, and rho (ignoring negative values).
      Number scale_factor = rho;
      if (0 <= rho_1 && rho_1 < scale_factor)
        scale_factor = rho_1;
      if (0 <= rho_2 && rho_2 < scale_factor)
        scale_factor = rho_2;
      return scale_factor;
    }
  }
}

} // namespace detail
} // namespace necklace_map
} // namespace geoviz
