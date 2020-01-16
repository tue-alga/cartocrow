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

#ifndef GEOVIZ_NECKLACE_MAP_DETAIL_GLYPH_SCALER_H
#define GEOVIZ_NECKLACE_MAP_DETAIL_GLYPH_SCALER_H

#include <memory>

#include "geoviz/common/core_types.h"
#include "geoviz/necklace_map/necklace.h"
#include "geoviz/necklace_map/map_element.h"


namespace geoviz
{
namespace necklace_map
{
namespace detail
{

// An 'event' node in the scaling process.
// As opposed to glyphs, these nodes may have a feasible interval completely outside [0, 2pi).
struct GlyphScalerNode
{
  using Ptr = std::shared_ptr<GlyphScalerNode>;

  GlyphScalerNode
  (
    const NecklaceGlyph::Ptr& glyph,
    const Number& covering_radius_dilated_rad
  );

  NecklaceGlyph::Ptr glyph;

  Number covering_radius_dilated_rad;

  // Note that unlike the glyph's feasible interval, these can be larger than 2*PI.
  Number feasible_angle_cw_rad;
  Number feasible_angle_ccw_rad;
}; // struct GlyphScalerNode


// Base functional glyph scaler, implementing simple recurring functions.
class GlyphScaler
{
 public:
  GlyphScaler(const Number& necklace_radius, const Number& dilation = 0);

  void AddNode(const NecklaceGlyph::Ptr& bead);

  virtual Number OptimizeScaleFactor() = 0;

 protected:
  void FinalizeNodes();

  // Number of nodes.
  size_t Size() const;

  // Clockwise extreme angle a_i of an interval.
  const Number& a(const size_t i) const;

  // Counterclockwise extreme angle b_i of an interval.
  const Number& b(const size_t i) const;

  // Covering radius r_i.
  const Number& r(const size_t i) const;

  // Aggregate covering radius r_ij.
  Number r(const size_t i, const size_t j) const;

  // Dual point l* to the line l describing the right extreme - scale factor relation of a glyph i to the left of the split index k.
  Point l_(const size_t i, const size_t k) const;

  // Dual point r* to the line r describing the left extreme - scale factor relation of a glyph j to the right of the split index k.
  Point r_(const size_t j, const size_t k) const;

  Number CorrectScaleFactor(const Number& rho) const;

 private:
  // Note that the scaler must be able to access the set by index.
  using NodeSet = std::vector<GlyphScalerNode>;
  NodeSet nodes_;

  Number necklace_radius_;
  Number dilation_;
};


// Functional glyph scaler for collections ordered by their interval.
class FixedGlyphScaler : public GlyphScaler
{
 public:
  FixedGlyphScaler(const Number& necklace_radius, const Number& dilation = 0);

  Number OptimizeScaleFactor();

 private:
  // Optimize the scale factor for the glyphs in the range [I, J].
  Number OptimizeSubProblem(const size_t I, const size_t J) const;
}; // class FixedGlyphScaler

} // namespace detail
} // namespace necklace_map
} // namespace geoviz

#endif //GEOVIZ_NECKLACE_MAP_DETAIL_GLYPH_SCALER_H
