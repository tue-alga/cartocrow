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

#ifndef GEOVIZ_NECKLACE_MAP_DETAIL_COMPUTE_SCALE_FACTOR_H
#define GEOVIZ_NECKLACE_MAP_DETAIL_COMPUTE_SCALE_FACTOR_H

#include <memory>

#include "geoviz/common/core_types.h"
#include "geoviz/necklace_map/bead.h"
#include "geoviz/necklace_map/necklace.h"
#include "geoviz/necklace_map/detail/cycle_node.h"


namespace geoviz
{
namespace necklace_map
{
namespace detail
{

class ComputeScaleFactor
{
 public:
  ComputeScaleFactor(const Necklace::Ptr& necklace, const Number& buffer_rad = 0);

  virtual Number Optimize() = 0;

  const Number& max_buffer_rad() const { return max_buffer_rad_; }

 protected:
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

  // Dual point l* to the line l describing the right extreme - scale factor relation of a bead i to the left of the split index k.
  Point l_(const size_t i, const size_t k) const;

  // Dual point r* to the line r describing the left extreme - scale factor relation of a bead j to the right of the split index k.
  Point r_(const size_t j, const size_t k) const;

  Number CorrectScaleFactor(const Number& rho) const;

 protected:
  Number max_buffer_rad_;

 private:
  // Note that the scaler must be able to access the set by index.
  using NodeSet = std::vector<BeadCycleNode>;
  NodeSet nodes_;

  Number necklace_radius_;
  Number buffer_rad_;
};


class ComputeScaleFactorFixedOrder : public ComputeScaleFactor
{
 public:
  ComputeScaleFactorFixedOrder(const Necklace::Ptr& necklace, const Number& buffer_rad = 0);

  Number Optimize();

 private:
  // Optimize the scale factor for the beads in the range [I, J].
  Number OptimizeSubProblem(const size_t I, const size_t J, Number& max_buffer_rad) const;
}; // class ComputeScaleFactorFixedOrder

} // namespace detail
} // namespace necklace_map
} // namespace geoviz

#endif //GEOVIZ_NECKLACE_MAP_DETAIL_COMPUTE_SCALE_FACTOR_H
