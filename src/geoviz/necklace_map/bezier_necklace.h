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

#include <array>
#include <vector>

#include "geoviz/common/core_types.h"
#include "geoviz/necklace_map/necklace_shape.h"


namespace geoviz
{
namespace necklace_map
{

class BezierCurve
{
 public:
  BezierCurve(const Point& source, const Point& control, const Point& target);
  BezierCurve(const Point& source, const Point& source_control, const Point& target_control, const Point& target);

  Point source() const;
  Point source_control() const;
  Point target_control() const;
  Point target() const;

  Point Evaluate(const Number& t) const;

  // There can be up to three intersections.
  bool IntersectRay(const Point& source, const Point& target, std::vector<Point>& intersections) const;

 protected:
  // Note that the control points are stored as vectors, because this simplifies most operations.
  std::array<Vector, 4> control_points_;
}; // class BezierCurve


class BezierNecklace : public NecklaceShape
{
 public:
  BezierNecklace(const Point& kernel);

  const Point& kernel() const;

  bool IsValid() const;

  void AppendCurve(const Point& source, const Point& source_control, const Point& target_control, const Point& target);

  void AppendCurve(const Point& source_control, const Point& target_control, const Point& target);

  void Finalize();

  Box ComputeBoundingBox() const;

  Number ComputeLength() const;
  Number ComputeRadius() const;
  Number ComputeCoveringSize(const Range::Ptr& range, const Number& radius) const;

  bool IntersectRay(const Number& angle_rad, Point& intersection) const;

  void Accept(NecklaceShapeVisitor& visitor);

 private:
  using CurveSet = std::vector<BezierCurve>;

  Point kernel_;

  CurveSet curves_;
}; // class BezierNecklace

} // namespace necklace_map
} // namespace geoviz

#endif //GEOVIZ_NECKLACE_MAP_BEZIER_NECKLACE_H
