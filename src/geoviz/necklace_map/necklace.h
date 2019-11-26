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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 07-11-19
*/

#ifndef GEOVIZ_NECKLACE_MAP_NECKLACE_H
#define GEOVIZ_NECKLACE_MAP_NECKLACE_H

#include "geoviz/common/core_types.h"


namespace geoviz
{
namespace necklace_map
{

struct NecklaceType
{
  virtual const Point& getKernel() const = 0;

  // TODO(tvl) should the necklace contain methods for adapting the 1D solution to the 2D solution?
  // This would mainly come into play for Bezier necklaces. For impl: check the 30x for loop in the Java prototype code.
  // Another case would be when a Glyph contains the necklace center;
  // in this case, the scale factor should be reduced such that the glyph does not contain the necklace center
  // (could this cause recursive failure and if so, in which cases? This *may* actually prove scientifically interesting).
}; // struct NecklaceType


struct CircleNecklace : public NecklaceType
{
  explicit CircleNecklace(const Circle& shape);
  const Point& getKernel() const;

  Circle shape;
}; // struct CircleNecklace


struct CurveNecklace : public CircleNecklace
{
  CurveNecklace(const Circle& shape, const Number& from_rad, const Number& to_rad);

  Number from_rad, to_rad;
}; // struct CurveNecklace


struct GeneralNecklace : public NecklaceType
{
  const Point& getKernel() const;

  Point center;
  std::vector<Point> points;
}; // struct GeneralNecklace

} // namespace necklace_map
} // namespace geoviz

#endif //GEOVIZ_NECKLACE_MAP_NECKLACE_H
