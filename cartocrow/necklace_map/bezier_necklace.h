/*
The Necklace Map library implements the algorithmic geo-visualization
method by the same name, developed by Bettina Speckmann and Kevin Verbeek
at TU Eindhoven (DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
Copyright (C) 2021  Netherlands eScience Center and TU Eindhoven

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

#ifndef CARTOCROW_NECKLACE_MAP_BEZIER_NECKLACE_H
#define CARTOCROW_NECKLACE_MAP_BEZIER_NECKLACE_H

#include "../core/core.h"
#include "../core/bezier.h"
#include "necklace_shape.h"

namespace cartocrow::necklace_map {

class BezierNecklace : public NecklaceShape {
  public:
	using Ptr = std::shared_ptr<BezierNecklace>;

	static constexpr const Number<Inexact> kDistanceRatioEpsilon = 1.001;

	BezierNecklace(const BezierSpline spline, const Point<Inexact>& kernel);

	const Point<Inexact>& kernel() const override;

	const BezierSpline& spline() const;

	bool isValid() const override;

	bool intersectRay(const Number<Inexact>& angle_rad, Point<Inexact>& intersection) const override;

	Box computeBoundingBox() const override;

	Number<Inexact> computeCoveringRadiusRad(const Range& range,
	                                         const Number<Inexact>& radius) const override;

	Number<Inexact> computeDistanceToKernel(const Range& range) const override;

	Number<Inexact> computeAngleAtDistanceRad(const Number<Inexact>& angle_rad,
	                                          const Number<Inexact>& distance) const override;

	void accept(NecklaceShapeVisitor& visitor) override;

  private:
	BezierSpline::CurveSet::const_iterator
	findCurveContainingAngle(const Number<Inexact>& angle_rad) const;

	bool intersectRay(const Number<Inexact>& angle_rad,
	                  const BezierSpline::CurveSet::const_iterator& curve_iter,
	                  Point<Inexact>& intersection, Number<Inexact>& t) const;

	bool computeAngleAtDistanceRad(const Point<Inexact>& point, const Number<Inexact>& distance,
	                               const BezierSpline::CurveSet::const_iterator& curve_point,
	                               const Number<Inexact>& t_point, Number<Inexact>& angle_rad) const;

	Number<Inexact> searchCurveForAngleAtDistanceRad(const Point<Inexact>& point,
	                                                 const BezierCurve& curve,
	                                                 const Number<Inexact>& squared_distance,
	                                                 const CGAL::Orientation& orientation,
	                                                 const Number<Inexact>& t_start) const;

	BezierSpline spline_;

	Point<Inexact> kernel_;
};

} // namespace cartocrow::necklace_map

#endif //CARTOCROW_NECKLACE_MAP_BEZIER_NECKLACE_H
