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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 28-11-2019
*/

#ifndef GEOVIZ_NECKLACE_MAP_GLYPH_SCALER_H
#define GEOVIZ_NECKLACE_MAP_GLYPH_SCALER_H

#include <unordered_map>
#include <vector>

#include "geoviz/common/core_types.h"
#include "geoviz/necklace_map/map_element.h"


namespace geoviz
{
namespace necklace_map
{

class GlyphScaler
{
 public:
  using Ordering = std::vector<size_t>;
  using NecklaceOrderMap = std::unordered_map<Necklace::Ptr, Ordering>;

  explicit GlyphScaler(const Number& min_separation = 0);

  // Note that elements with value 0 will not be included in the ordering.
  virtual Number operator()(const std::vector<MapElement::Ptr>& elements, NecklaceOrderMap& ordering) const = 0;

 protected:
  Number dilation_;
}; // class GlyphScaler


class GlyphScalerFixedOrder : public GlyphScaler
{
 public:
  explicit GlyphScalerFixedOrder(const Number& min_separation = 0);

  Number operator()(const std::vector<MapElement::Ptr>& elements, NecklaceOrderMap& ordering) const;
}; // class GlyphScalerFixedOrder

} // namespace necklace_map
} // namespace geoviz

#endif //GEOVIZ_NECKLACE_MAP_GLYPH_SCALER_H
