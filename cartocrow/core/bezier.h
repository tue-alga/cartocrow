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

#ifndef CARTOCROW_NECKLACEMAP_BEZIER_H
#define CARTOCROW_NECKLACEMAP_BEZIER_H

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
	BezierCurve(const Point<Inexact>& source, const Point<Inexact>& source_control,
	            const Point<Inexact>& target_control, const Point<Inexact>& target);

	/// Constructs a cubic Bezier curve from a quadratic Bezier curve consisting of three control points.
	BezierCurve(const Point<Inexact>& source, const Point<Inexact>& control, const Point<Inexact>& target);

	/// Returns the source of this curve.
	Point<Inexact> source() const;
	/// Returns the control point on the source side of this curve.
	Point<Inexact> sourceControl() const;
	/// Returns the control point on the target side of this curve.
	Point<Inexact> targetControl() const;
	/// Returns the target of this curve.
	Point<Inexact> target() const;

	/// Evaluates the curve at time \c t.
	Point<Inexact> evaluate(const Number<Inexact>& t) const;

	/// TODO document
	// There can be up to three intersections.
	size_t intersectRay(const Point<Inexact>& source, const Point<Inexact>& target,
	                    Point<Inexact>* intersections, Number<Inexact>* intersection_t) const;

	BezierCurve transform(const CGAL::Aff_transformation_2<Inexact> &t) const;

  protected:
	// TODO control points stored as Vectors instead of Points
	std::array<Vector<Inexact>, 4> m_controlPoints;
	std::array<Vector<Inexact>, 4> m_coefficients;
};

class BezierSpline {
  public:
	using CurveSet = std::vector<BezierCurve>;

	BezierSpline();

	bool isValid() const;

	bool isEmpty() const;

	bool isContinuous() const;

	bool isClosed() const;

	bool toCircle(Circle<Inexact>& circle, const Number<Inexact>& epsilon = 0.01) const;

	const CurveSet& curves() const;

	CurveSet& curves();

	void appendCurve(const Point<Inexact>& source, const Point<Inexact>& source_control,
	                 const Point<Inexact>& target_control, const Point<Inexact>& target);

	void appendCurve(BezierCurve curve);

	void appendCurve(const Point<Inexact>& source_control, const Point<Inexact>& target_control,
	                 const Point<Inexact>& target);

	void reverse();

	Box computeBoundingBox() const;

  private:
	CurveSet curves_;

	mutable Box bounding_box_;
};

} // namespace cartocrow::necklace_map

#endif //CARTOCROW_NECKLACEMAP_BEZIER_SPLINE_H
