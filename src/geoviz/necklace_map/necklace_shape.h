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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 15-01-2020
*/

#ifndef GEOVIZ_NECKLACE_MAP_NECKLACE_SHAPE_H
#define GEOVIZ_NECKLACE_MAP_NECKLACE_SHAPE_H

#include <memory>

#include "geoviz/common/core_types.h"
#include "geoviz/necklace_map/necklace_interval.h"


namespace geoviz
{
namespace necklace_map
{

class CircleNecklace;
class BezierNecklace;


class NecklaceShapeVisitor
{
 public:
  virtual void Visit(CircleNecklace& shape) {}
  virtual void Visit(BezierNecklace& shape) {}
}; // class NecklaceShapeVisitor


class NecklaceShape
{
 public:
  using Ptr = std::shared_ptr<NecklaceShape>;

  virtual const Point& kernel() const = 0;

  virtual bool IsValid() const = 0;

  virtual Box ComputeBoundingBox() const = 0;

  virtual Number ComputeLength() const = 0;
  virtual Number ComputeRadius() const = 0;  // TODO(tvl) replace by ComputeLength()?

  virtual Number ComputeCoveringRadius(const Range::Ptr& range, const Number& radius) const = 0; // TODO(tvl) rename "ComputeCoveringRadius" and document.

  virtual bool IntersectRay(const Number& angle_rad, Point& intersection) const = 0;

  Number ComputeAngle(const Point& point) const;

  virtual void Accept(NecklaceShapeVisitor& visitor) = 0;
  // TODO(tvl) should the necklace contain methods for adapting the 1D solution to the 2D solution?
  // This would mainly come into play for Bezier necklaces. For impl: check the 30x for loop in the Java prototype code.
  // Another case would be when a bead contains the necklace center;
  // in this case, the scale factor should be reduced such that the bead does not contain the necklace center
  // (could this cause recursive failure and if so, in which cases? This *may* actually prove scientifically interesting).
}; // class NecklaceShape

} // namespace necklace_map
} // namespace geoviz

#endif //GEOVIZ_NECKLACE_MAP_NECKLACE_SHAPE_H
