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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 07-11-2019
*/

#ifndef GEOVIZ_NECKLACE_MAP_NECKLACE_H
#define GEOVIZ_NECKLACE_MAP_NECKLACE_H

#include "geoviz/common/core_types.h"


namespace geoviz
{
namespace necklace_map
{

class NecklaceType
{
 public:
  virtual const Point& kernel() const = 0;
  virtual bool IntersectRay(const Number& angle_rad, Point& intersection) const = 0;
  virtual Box ComputeBoundingBox() const = 0;

  // TODO(tvl) should the necklace contain methods for adapting the 1D solution to the 2D solution?
  // This would mainly come into play for Bezier necklaces. For impl: check the 30x for loop in the Java prototype code.
  // Another case would be when a Glyph contains the necklace center;
  // in this case, the scale factor should be reduced such that the glyph does not contain the necklace center
  // (could this cause recursive failure and if so, in which cases? This *may* actually prove scientifically interesting).
}; // class NecklaceType


class CircleNecklace : public NecklaceType
{
 public:
  explicit CircleNecklace(const Circle& shape);
  const Point& kernel() const;
  bool IntersectRay(const Number& angle_rad, Point& intersection) const;
  Box ComputeBoundingBox() const;

 protected:
  Circle shape_;
}; // class CircleNecklace


class CurveNecklace : public CircleNecklace
{
 public:
  CurveNecklace(const Circle& shape, const Number& angle_cw_rad, const Number& angle_ccw_rad);
  bool IntersectRay(const Number& angle_rad, Point& intersection) const;

 private:
  Number angle_cw_rad_; // Internally, this angle is adjusted to be in the range [0, 2*pi).
  Number angle_ccw_rad_; // For convenience, this angle is adjusted to be in the range [angle_from_rad, angle_from_rad+2*pi).
}; // class CurveNecklace


class GenericNecklace : public NecklaceType
{
 public:
  const Point& getKernel() const;
  bool IntersectRay(const Number& angle_rad, Point& intersection) const;
  Box ComputeBoundingBox() const;

 private:
  Point kernel_;
  std::vector<Point> points_;  // TODO(tvl) the geneic necklace will probably require different markers than just points.
}; // class GenericNecklace

} // namespace necklace_map
} // namespace geoviz

#endif //GEOVIZ_NECKLACE_MAP_NECKLACE_H
