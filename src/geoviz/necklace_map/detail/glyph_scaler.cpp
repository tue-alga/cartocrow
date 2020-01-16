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

#include "glyph_scaler.h"

#include <glog/logging.h>


namespace geoviz
{
namespace necklace_map
{
namespace detail
{

GlyphScalerNode::GlyphScalerNode
(
  const NecklaceGlyph::Ptr& glyph,
  const Number& covering_radius_dilated_rad
) : glyph(glyph),
    covering_radius_dilated_rad(covering_radius_dilated_rad),
    feasible_angle_cw_rad(glyph->interval->angle_cw_rad()),
    feasible_angle_ccw_rad(glyph->interval->angle_ccw_rad())
{}


GlyphScaler::GlyphScaler(const Number& necklace_radius, const Number& dilation /*= 0*/)
  : nodes_(), necklace_radius_(necklace_radius), dilation_(dilation)
{}

void GlyphScaler::AddNode(const NecklaceGlyph::Ptr& bead)
{
  CHECK_GT(bead->radius_base, 0);
  const Number covering_radius_dilated_rad = std::asin((bead->radius_base + dilation_) / necklace_radius_);
  // Note that for an exact computation, the scaling factor should be inside this arcsine function.
  // This can be solved by performing a bisection search on the scale factors using a feasibility check to see if the scaled glyphs fit.

  nodes_.emplace_back(bead, covering_radius_dilated_rad);
}

void GlyphScaler::FinalizeNodes()
{
  if (nodes_.empty() || nodes_[0].glyph == nodes_[Size()/2].glyph)
    return;
  nodes_.reserve(2 * nodes_.size());

  // Each node is duplicated with an offset to its feasible interval to force cyclic validity.
  const NodeSet::iterator end = nodes_.end();
  for (NodeSet::iterator node_iter = nodes_.begin(); node_iter != end; ++node_iter)
  {
    CHECK_NOTNULL(node_iter->glyph);
    nodes_.emplace_back(node_iter->glyph, node_iter->covering_radius_dilated_rad);
    nodes_.back().feasible_angle_cw_rad += M_2xPI;
    nodes_.back().feasible_angle_ccw_rad += M_2xPI;
  }
}

inline size_t GlyphScaler::Size() const
{
  return nodes_.size();
}

// Interval start a_i.
inline const Number& GlyphScaler::a(const size_t i) const
{
  return nodes_[i].feasible_angle_cw_rad;
}

// Interval end b_i.
inline const Number& GlyphScaler::b(const size_t i) const
{
  return nodes_[i].feasible_angle_ccw_rad;
}

// Radius r_i.
inline const Number& GlyphScaler::r(const size_t i) const
{
  return nodes_[i].covering_radius_dilated_rad;
}

Number GlyphScaler::r(const size_t i, const size_t j) const
{
  // Note that we could store (partial) results, but the gains would be minimal.
  Number aggregate_radius = 0;
  for (size_t n = i; n <= j; ++n)
    aggregate_radius += r(n);
  return aggregate_radius;
}

inline Point GlyphScaler::l_(const size_t i, const size_t k) const
{
  Number x = 1 / (2 * r(i, k) - r(i));
  CHECK_GE(x, 0);
  return Point(x, a(i) * x);
}

inline Point GlyphScaler::r_(const size_t j, const size_t k) const
{
  Number x = -1 / (2 * r(k + 1, j) - r(j));
  CHECK_LE(x, 0);
  return Point(x, b(j) * x);
}

Number GlyphScaler::CorrectScaleFactor(const Number& rho) const
{
  // Determine a lower bound on the scale factor by reverse engineering based on the dilated covering radius.
  // Note that while this forces the scale factor to be such that none of the scaled glyphs cover more than their scaled covering radius, the scale factor may often be increased slightly to exploit the freed up space on the scaled covering radius of the glyph's neighbors.
  Number scale_factor = rho;
  for (size_t n = 0; n < Size(); ++n)
  {
    const Number rho_prime =
      (necklace_radius_ * std::sin(rho * nodes_[n].covering_radius_dilated_rad) - dilation_) / nodes_[n].glyph->radius_base;
    if (rho_prime < scale_factor)
      scale_factor = rho_prime;
  }
  return scale_factor;
}


FixedGlyphScaler::FixedGlyphScaler(const Number& necklace_radius, const Number& dilation /*= 0*/) : GlyphScaler(necklace_radius, dilation) {}

Number FixedGlyphScaler::OptimizeScaleFactor()
{
  FinalizeNodes();

  const Number rho = OptimizeSubProblem(0, Size() - 1);
  const Number rho_fill_circle = M_PI / r(0, (Size() / 2) - 1);  // Note that the necklace glyphs were added twice.
  return rho < 0 ? rho_fill_circle : std::min(CorrectScaleFactor(rho), rho_fill_circle);
}

// Note that this computes the subproblem including J (unlike C++ customs defining the end as one-past).
Number FixedGlyphScaler::OptimizeSubProblem(const size_t I, const size_t J) const
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
      return (b(J) - a(I)) / (r(I) + r(J));  // rho_IJ = (b_J - a_I) / (2*r_IJ - r_I - r_J)
    default:
    {
      // Compute the scale factor using divide-and-conquer:
      // split the problem into two sub-problems with roughly half the size.
      const size_t k = (I + J) / 2;
      const Number rho_1 = OptimizeSubProblem(I, k);
      const Number rho_2 = OptimizeSubProblem(k+1, J);

      // For the conquer part, we also need the smallest rho_ij where I <= i <= k < j <= J.
      // This smallest rho_ij is the lowest intersection (over all i,j | i <= k < j) of l_i, r_j,
      // where l_i = (X - a_i) / (2*r_ik - r_i) and r_j = (b_j - X) / (2*r_mj - r_j)
      // [so rho_ij = (b_j - a_i) / (2*r_ik - r_i + 2*r_mj - r_j)] = (b_j - a_i) / (2*r_ij - r_i - r_j).
      // This lowest intersection is the the line of the upper envelope of {L' union R'} that intersects the y axis,
      // where L' is the set of points l'_i = <1 / (2*r_ik - r_i), a_i / (2*r_ik - r_i)>,
      // and R' is the set of points r'_i = <-1 / (2*r_mj - r_j), -b_j / (2*r_mj - r_j)>.

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

      const Number rho = (b(jj) - a(ii)) / (2*r(ii, jj) - r(ii) - r(jj));
      CHECK_GT(rho, 0);

      // Alternatively, we could compute all O(n^2) combinations of i and j.
      /*Number rho = -1;
      for (size_t i = I; i <= k; ++i)
      {
        for (size_t j = m; j <= J; ++j)
        {
          const Number rho_ij = (b(j) - a(i)) / (2 * r(i, j) - r(i) - r(j));
          if (rho == -1 || rho_ij < rho)
            rho = rho_ij;
          CHECK_GT(rho, 0);
        }
      }*/

      // The scaling factor is the minimum or rho_1, rho_2, and rho (ignoring negative values).
      Number scale_factor = rho;
      if (0 < rho_1 && rho_1 < scale_factor)
        scale_factor = rho_1;
      if (0 < rho_2 && rho_2 < scale_factor)
        scale_factor = rho_2;
      return scale_factor;
    }
  }
}

} // namespace detail
} // namespace necklace_map
} // namespace geoviz
