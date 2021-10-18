/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 08-09-2020
*/

#ifndef CARTOCROW_CORE_BEZIER_SPLINE_H
#define CARTOCROW_CORE_BEZIER_SPLINE_H

#include <array>
#include <vector>

#include "cartocrow/core/core_types.h"

namespace cartocrow {

class BezierCurve {
  public:
	BezierCurve(const Point& source, const Point& control, const Point& target);
	BezierCurve(const Point& source, const Point& source_control, const Point& target_control,
	            const Point& target);

	Point source() const;
	Point source_control() const;
	Point target_control() const;
	Point target() const;

	Point Evaluate(const Number& t) const;

	// There can be up to three intersections.
	size_t IntersectRay(const Point& source, const Point& target, Point* intersections,
	                    Number* intersection_t) const;

  protected:
	// Note that the control points are stored as vectors, because this simplifies most operations.
	std::array<Vector, 4> control_points_;
	std::array<Vector, 4> coefficients_;
}; // class BezierCurve

class BezierSpline {
  public:
	using CurveSet = std::vector<BezierCurve>;

	BezierSpline();

	bool IsValid() const;

	bool IsEmpty() const;

	bool IsContinuous() const;

	bool IsClosed() const;

	bool ToCircle(Circle& circle, const Number& epsilon = 0.01) const;

	const CurveSet& curves() const;

	CurveSet& curves();

	void AppendCurve(const Point& source, const Point& source_control, const Point& target_control,
	                 const Point& target);

	void AppendCurve(const Point& source_control, const Point& target_control, const Point& target);

	void Reverse();

	Box ComputeBoundingBox() const;

  private:
	CurveSet curves_;

	mutable Box bounding_box_;
}; // class BezierSpline

} // namespace cartocrow

#endif //CARTOCROW_CORE_BEZIER_SPLINE_H
