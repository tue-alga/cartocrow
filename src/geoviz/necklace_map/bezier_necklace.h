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
namespace necklace_map  // TODO(tvl) move into geoviz/common and rename BezierNecklace to "BezierSpline" (separate bezier construction and bbox / covering radius / angle computations).
{

class BezierCurve
{
 public:
  BezierCurve(const Point& source, const Point& control, const Point& target);
  BezierCurve(const Point& source, const Point& source_control, const Point& target_control, const Point& target);

  bool IsValid(const Point& kernel) const;

  Point source() const;
  Point source_control() const;
  Point target_control() const;
  Point target() const;

  Point Evaluate(const Number& t) const;

  // There can be up to three intersections.
  size_t IntersectRay(const Point& source, const Point& target, Point* intersections, Number* intersection_t) const;

 protected:
  // Note that the control points are stored as vectors, because this simplifies most operations.
  std::array<Vector, 4> control_points_;
  std::array<Vector, 4> coefficients_;
}; // class BezierCurve


class BezierNecklaceVisitor : public NecklaceShapeVisitor
{
 public:
  virtual void Visit(BezierCurve& curve) {}
}; // class BezierNecklaceVisitor


class BezierNecklace : public NecklaceShape
{
 public:
  using Ptr = std::shared_ptr<BezierNecklace>;

  static constexpr const Number kDistanceRatioEpsilon = 1.001;

  BezierNecklace(const Point& kernel);

  const Point& kernel() const override;

  bool IsValid() const override;

  bool IsEmpty() const override;

  bool IsClosed() const override;

  bool IntersectRay(const Number& angle_rad, Point& intersection) const override;

  void AppendCurve(const Point& source, const Point& source_control, const Point& target_control, const Point& target);

  void AppendCurve(const Point& source_control, const Point& target_control, const Point& target);

  void Finalize();

  Box ComputeBoundingBox() const override;

  Number ComputeCoveringRadiusRad(const Range::Ptr& range, const Number& radius) const override;

  Number ComputeAngleAtDistanceRad(const Number& angle_rad, const Number& distance) const override;

  void Accept(NecklaceShapeVisitor& visitor) override;

  void IterateCurves(BezierNecklaceVisitor& visitor);

 private:
  using CurveSet = std::vector<BezierCurve>;

  CurveSet::const_iterator FindCurveContainingAngle(const Number& angle_rad) const;

  bool IntersectRay(const Number& angle_rad, const CurveSet::const_iterator& curve_iter, Point& intersection, Number& t) const;

  bool ComputeAngleAtDistanceRad
  (
    const Point& point,
    const Number& distance,
    const CurveSet::const_iterator& curve_point,
    const Number& t_point,
    Number& angle_rad
  ) const;

  Number SearchCurveForAngleAtDistanceRad
  (
    const Point& point,
    const BezierCurve& curve,
    const Number& squared_distance,
    const CGAL::Orientation& orientation,
    const Number& t_start
  ) const;

  Point kernel_;

  bool checked_;  // TODO(tvl) tmp DEBUG!

  CurveSet curves_;

  CGAL::Orientation winding_;

  mutable Box bounding_box_;
}; // class BezierNecklace

} // namespace necklace_map
} // namespace geoviz

#endif //GEOVIZ_NECKLACE_MAP_BEZIER_NECKLACE_H
