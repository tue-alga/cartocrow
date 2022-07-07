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

#include "../core/core.h"

namespace cartocrow {

/// A cubic Bezier curve.
/**
 * A Bezier curve is defined by four points: the source, the target, and two
 * control points.
 */
class BezierCurve {
  public:
	/// Constructs a cubic Bezier curve based on four control points.
	BezierCurve(const Point& source, const Point& source_control, const Point& target_control,
	            const Point& target);

	/// Returns the source of this curve.
	Point source() const;
	/// Returns the control point on the source side of this curve.
	Point sourceControl() const;
	/// Returns the control point on the target side of this curve.
	Point targetControl() const;
	/// Returns the target of this curve.
	Point target() const;

	/// Evaluates the curve at time \c t.
	Point evaluate(const Number& t) const;

	/// There can be up to three intersections.
	size_t intersectRay(const Point& source, const Point& target, Point* intersections,
	                    Number* intersection_t) const;

  protected:
	std::array<Point, 4> m_controlPoints;
	std::array<Point, 4> m_coefficients;
};

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
