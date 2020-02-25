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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 25-02-2020
*/

#ifndef GEOVIZ_NECKLACE_MAP_BEZIER_NECKLACE_H
#define GEOVIZ_NECKLACE_MAP_BEZIER_NECKLACE_H

#include <vector>

#include "geoviz/common/core_types.h"
#include "geoviz/necklace_map/necklace_shape.h"


namespace geoviz
{
namespace necklace_map
{

class BezierNecklace : public NecklaceShape
{
 public:
  const Point& kernel() const;
  bool IntersectRay(const Number& angle_rad, Point& intersection) const;
  Box ComputeBoundingBox() const;
  //Number ComputeLength() const;
  Number ComputeRadius() const;

  void Accept(NecklaceShapeVisitor& visitor);

 private:
  Point kernel_;
  std::vector<Point> points_;  // TODO(tvl) the generic necklace will probably require different markers than just points.
  Number radius_;
}; // class BezierNecklace

} // namespace necklace_map
} // namespace geoviz

#endif //GEOVIZ_NECKLACE_MAP_BEZIER_NECKLACE_H
