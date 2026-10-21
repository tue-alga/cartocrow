/*
Copyright (C) 2026  TU Eindhoven

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
*/

#pragma once

#include "core.h"
#include "polyline.h"
#include "root_finding_helpers.h"

#include <CGAL/Arr_Bezier_curve_traits_2.h>
#include <CGAL/Cartesian.h>
#include <CGAL/CORE_algebraic_number_traits.h>
#include <CGAL/Arr_Bezier_curve_traits_2.h>

#include <CGAL/Surface_sweep_2_algorithms.h>

#include <ranges>

namespace cartocrow {
class CubicBezierSpline;

namespace detail {
template <class CubicBezierCurveOrSpline1, class CubicBezierCurveOrSpline2>
bool intersectsRecursive(const CubicBezierCurveOrSpline1& c1, const CubicBezierCurveOrSpline2& c2, double threshold) {
	auto c1Box = c1.bbox();
	auto c2Box = c2.bbox();
	if (!do_overlap(c1Box, c2Box)) return false;
	auto comb = c1Box + c2Box;
	if (std::max(comb.x_span(), comb.y_span()) < threshold) {
		return true;
	}
	auto [c11, c12] = c1.split(0.5);
	auto [c21, c22] = c2.split(0.5);
	for (auto& c1x : {c11, c12}) {
		for (auto& c2x : {c21, c22}) {
			if (intersectsRecursive(c1x, c2x, threshold)) return true;
		}
	}
	return false;
}

template <class CubicBezierCurveOrSpline1, class CubicBezierCurveOrSpline2, class OutputIterator>
void intersectionsRecursive(const CubicBezierCurveOrSpline1& c1, const CubicBezierCurveOrSpline2& c2, OutputIterator out, 
							double c1_t0, double c1_t1, double c2_t0, double c2_t1, double threshold) {
	auto c1Box = c1.bbox();
	auto c2Box = c2.bbox();
	if (!do_overlap(c1Box, c2Box))
		return;
	auto comb = c1Box + c2Box;
	if (std::max(comb.x_span(), comb.y_span()) < threshold) {
		*out++ = {c1.source(), c1.parameter(c1_t0), c2.parameter(c2_t0)};
		return;
	}
	auto [c11, c12] = c1.split(0.5);
	auto [c21, c22] = c2.split(0.5);
	auto c1_tm = (c1_t0 + c1_t1) / 2;
	auto c2_tm = (c2_t0 + c2_t1) / 2;

	intersectionsRecursive(c11, c21, out, c1_t0, c1_tm, c2_t0, c2_tm, threshold);
	intersectionsRecursive(c11, c22, out, c1_t0, c1_tm, c2_tm, c2_t1, threshold);
	intersectionsRecursive(c12, c21, out, c1_tm, c1_t1, c2_t0, c2_tm, threshold);
	intersectionsRecursive(c12, c22, out, c1_tm, c1_t1, c2_tm, c2_t1, threshold);
}
}

template <class CubicBezierCurveOrSpline1, class CubicBezierCurveOrSpline2>
struct CubicBezierIntersectionResult {
	Point<Inexact> point;
	typename CubicBezierCurveOrSpline1::Parameter parameter1;
	typename CubicBezierCurveOrSpline2::Parameter parameter2;
};

/// A cubic Bézier curve.
/// Cubic Bézier curves can be combined to form a cubic Bézier spline (\ref CubicBezierSpline).
class CubicBezierCurve {
  public:
	using K = Inexact;

  private:
	/// Zeroth control point aka source
	Point<K> m_p0;
	/// First control point
	Point<K> m_p1;
	/// Second control point
	Point<K> m_p2;
	/// Third control point aka target
	Point<K> m_p3;

	/// Zeroth control point in the form of a vector (for convenience)
	Vector<K> m_v0;
	/// First control point in the form of a vector (for convenience)
	Vector<K> m_v1;
	/// Second control point in the form of a vector (for convenience)
	Vector<K> m_v2;
	/// Third control point in the form of a vector (for convenience)
	Vector<K> m_v3;

  public:
	bool operator==(const CubicBezierCurve& other) const {
		return m_p0 == other.m_p0 && m_p1 == other.m_p1 && m_p2 == other.m_p2 && m_p3 == other.m_p3;
	}

	/// Construct a cubic Bézier curve from its two endpoints and two control points.
	CubicBezierCurve(Point<K> source, Point<K> control1, Point<K> control2, Point<K> target);

	/// Construct a cubic Bézier curve from two endpoints and one control point (i.e. from a quadratic Bézier curve).
	CubicBezierCurve(Point<K> source, Point<K> control, Point<K> target);

	/// Construct a cubic Bézier curve from two endpoints.
	CubicBezierCurve(Point<K> source, Point<K> target);

	/// Construct a cubic Bézier curve from a line segment.
	CubicBezierCurve(Segment<K> seg);

	/// Returns the source of this curve.
	Point<K> source() const;
	/// Returns the control point on the source side of this curve.
	Point<K> sourceControl() const;
	/// Returns the control point on the target side of this curve.
	Point<K> targetControl() const;
	/// Returns the target of this curve.
	Point<K> target() const;
	/// Returns the ith control point.
	/// \pre 0 \<= i \<= 3
	Point<K> control(int i) const;

	/// Evaluates the curve at time \c t.
	Point<K> evaluate(const Number<K>& t) const;
	/// Evaluates the curve at time \c t.
	Point<K> position(const Number<K>& t) const;
	/// Evaluates the derivative at time \c t.
	Vector<K> derivative(const Number<K>& t) const;
	/// Evaluates the second derivative at time \c t.
	Vector<K> derivative2(const Number<K>& t) const;
	/// Computes the tangent at time \c t.
	Vector<K> tangent(const Number<K>& t) const;
	/// Computes the normal at time \c t.
	Vector<K> normal(const Number<K>& t) const;
	/// Computes the curvature at time \c t.
	Number<K> curvature(Number<K> t) const;

	/// Computes the signed area of the curve.
	/// Positive for counter-clockwise curves, negative otherwise.
	/// For open curves it returns the signed area as if the curve was closed with a line segment between the endpoints.
	Number<K> signedArea() const;

	/// Return the reverse of this Bézier curve
	CubicBezierCurve reversed() const;
	/// Reverse this Bézier curve
	void reverse();

	/// Returns the two parts after splitting this Bézier curve at point \c p at time \c t.
	/// That is, the first curve of this pair starts at the source and ends at \c p.
	/// The second curve of this pair starts at \c p and ends at the target.
	/// Note that no approximation is needed for this operation, the curves match the original exactly (up to floating-point errors).
	std::pair<CubicBezierCurve, CubicBezierCurve> split(const Number<K>& t) const;

	/// Returns the part of this Bézier curve between parameters \c t1 and \c t2.
	CubicBezierCurve sub(const Number<K>& t1, const Number<K>& t2) const;

	/// Sample points with equidistant parameter values (not equidistant sample points).
	template <class OutputIterator>
	void samplePoints(int nPoints, OutputIterator out) const {
		double step = 1.0 / (nPoints - 1);
		for (int i = 0; i < nPoints; ++i) {
			double t = i * step;
			*out++ = evaluate(t);
		}
	}

	/// Returns a naive approximation of this Bézier curve by a polyline with nEdges.
	/// The polyline starts at the source and ends at the target of the curve.
	/// All vertices of the polyline lie on the Bézier curve and their parameter values (not the points) are equidistant
	Polyline<K> polyline(int nEdges) const;

	/// Return a transformed version of the Bézier curve.
	CubicBezierCurve transform(const CGAL::Aff_transformation_2<Inexact> &t) const;

	using Parameter = Number<K>;

	Parameter parameter(double param) const;
	double doubleParameter(Parameter param) const;

	struct CurvePoint {
		Number<K> t;
		Point<K> point;
	};

	/// Given a point, return the nearest point on the curve together with its parameter value.
	CurvePoint nearest(Point<K> point, double threshold = M_EPSILON) const;

	/// Returns the extrema on the curve: left-, bottom-, right-, top-most points on the curve.
	std::tuple<CurvePoint, CurvePoint, CurvePoint, CurvePoint> extrema() const;

	/// Returns the axis-aligned bounding box.
	Box bbox() const;

	/// Returns the coefficients of the polynomial expression of the parameterized curve.
	/// at³ + bt² + ct + d
	/// \returns (a, b, c, d)
	std::tuple<Vector<K>, Vector<K>, Vector<K>, Vector<K>> coefficients() const;

	/// Outputs the \c t values at which the Bézier intersects the given line.
	template <class OutputIterator>
	void intersectionsT(const Line<K>& line, OutputIterator out) const {
		Vector<K> ab(line.a(), line.b());
		auto [c0, c1, c2, c3] = coefficients();
		getCubicRoots(c0 * ab, c1 * ab, c2 * ab, c3 * ab + line.c(), out, true);
	}

	/// Outputs the curve points (\ref CurvePoint) at which the Bézier intersects the given line.
	template <class OutputIterator>
	void intersections(const Line<K>& line, OutputIterator out) const {
		std::vector<Number<K>> intersT;
		intersectionsT(line, std::back_inserter(intersT));
		for (Number<K> t : intersT) {
			*out++ = CurvePoint{t, evaluate(t)};
		}
	}

	/// Outputs the curve points (\ref CurvePoint) at which the Bézier intersects the given line segment.
	template <class OutputIterator>
	void intersections(const Segment<K>& segment, OutputIterator out) const {
		std::vector<Number<K>> intersT;
		intersectionsT(segment.supporting_line(), std::back_inserter(intersT));
		for (Number<K> t : intersT) {
			auto point = evaluate(t);
			// Parameter on segment
			auto s = (point - segment.source()) * (segment.target() - segment.source()) / segment.squared_length();
			if (s >= 0 && s <= 1) {
				*out++ = CurvePoint{t, point};
			}
		}
	}

	/// Outputs the curve points (\ref CurvePoint) at which the Bézier intersects the given ray.
	template <class OutputIterator>
	void intersections(const Ray<K>& ray, OutputIterator out) const {
		std::vector<Number<K>> intersT;
		intersectionsT(ray.supporting_line(), std::back_inserter(intersT));

		for (Number<K> t : intersT) {
			auto point = evaluate(t);
			auto v = ray.to_vector();
			// Parameter on ray
			auto s = (point - ray.source()) * v / v.squared_length();
			if (s >= 0) {
				*out++ = CurvePoint{t, point};
			}
		}
	}

  public:
	template <class OutputIterator>
	void monotoneParts(OutputIterator out) const {
		auto a = 3 * (-m_v0 + 3 * m_v1 - 3 * m_v2 + m_v3);
		auto b = 6 * (m_v0 - 2 * m_v1 + m_v2);
		auto c = 3 * (m_v1 - m_v0);

		std::vector<double> ts({0, 1});

		auto checkExtrema = [&ts](double A, double B, double C) {
			double D = B * B - 4 * A * C;
			if (D < 0)
				return;
			double sqrtD = std::sqrt(D);
			double denom = 2 * A;
			if (denom == 0)
				return;

			for (double t : {(-B - sqrtD) / denom, (-B + sqrtD) / denom}) {
				if (t >= 0.0 && t <= 1.0) {
					ts.push_back(t);
				}
			}
		};
		checkExtrema(a.x(), b.x(), c.x());
		checkExtrema(a.y(), b.y(), c.y());

		std::sort(ts.begin(), ts.end());
		ts.erase(std::unique(ts.begin(), ts.end()), ts.end());
		for (int ti = 0; ti < ts.size() - 1; ++ti) {
			double t1 = ts[ti];
			double t2 = ts[ti + 1];
			if (t2 - t1 < M_EPSILON) continue;
		    *out++ = sub(t1, t2);
		}
	}

	/// Outputs whether the two Bézier curves intersect.
	/// This is approximate controlled by threshold (the max. dimension of the rectangle in which curves are assumed to intersect).
	bool intersects(const CubicBezierCurve& other, double threshold = M_EPSILON) const;
	bool intersects(const CubicBezierSpline& other, double threshold = M_EPSILON) const;
	bool selfIntersects(double threshold = M_EPSILON) const;

	/// Outputs the parameter values (doubles) at which the curvature flips sign.
	template <class OutputIterator>
	void inflectionsT(OutputIterator out) const {
		auto cross = [](const Vector<K>& u, const Vector<K>& v) {
			return u.x() * v.y() - u.y() * v.x();
		};

		auto [a, b, c, d] = coefficients();
		auto A = 3 * cross(a, b);
		auto B = 3 * cross(a, c);
		auto C = cross(b, c);

		getQuadraticRoots(A, B, C, out, true);
	}

	/// Outputs the curve points (\ref CurvePoint) at which the curvature flips sign.
	template <class OutputIterator>
	void inflections(OutputIterator out) const {
		std::vector<Number<K>> ts;
		inflectionsT(std::back_inserter(ts));
		for (const auto& t : ts) {
			*out++ = CurvePoint{t, evaluate(t)};
		}
	}
};

/// A cubic Bézier spline.
/// It consists of a sequence of cubic Bézier curves that share endpoints and in that way form a G^0 continuous curve.
class CubicBezierSpline {
  public:
	using Curve = CubicBezierCurve;
	using Spline = CubicBezierSpline;
	using K = Inexact;
	using ControlsContainer = std::vector<Point<K>>;
  private:
	/// Control points. Its size, if non-empty, is 3k+1 where k is the number of curves.
	ControlsContainer m_c;

  public:
	bool operator==(const CubicBezierSpline& other) const {
		return m_c == other.m_c;
	}

	// === Creation ===
	/// Create an empty spline.
	CubicBezierSpline();

	/// Create a spline from a sequence of 3k+1 control points (Point<Inexact>).
	template <class InputIterator>
	CubicBezierSpline(InputIterator begin, InputIterator end) : m_c(begin, end)
		{ assert(m_c.size() == 0 || (m_c.size() - 1) % 3 == 0);}

	/// Create a spline from a polyline.
	CubicBezierSpline(Segment<K> segment);

	/// Create a spline from a single curve;
	CubicBezierSpline(CubicBezierCurve curve);

	/// Create a spline from a polyline.
	CubicBezierSpline(Polyline<K> polyline);

	/// Append a cubic Bézier curve.
	void appendCurve(const Curve& curve);
	/// Append a cubic Bézier curve from its two endpoints and two control points.
	void appendCurve(Point<K> source, Point<K> control1, Point<K> control2, Point<K> target);
	/// Append a cubic Bézier curve from two endpoints and one control point (i.e. from a quadratic Bézier curve).
	void appendCurve(Point<K> source, Point<K> control, Point<K> target);
	/// Append a cubic Bézier curve from two endpoints.
	void appendCurve(Point<K> source, Point<K> target);
	/// Append a cubic Bézier spline.
	void appendSpline(const CubicBezierSpline& spline);

	// === Helper structs ===
	/// A Bézier spline is parameterized by a pair of a curve index and a curve parameter.
	struct SplineParameter {
		int curveIndex;
		Number<K> t;

		bool operator==(const SplineParameter& other) const {
			return curveIndex == other.curveIndex && t == other.t;
		}

		bool operator!=(const SplineParameter& other) const {
			return curveIndex != other.curveIndex || t != other.t;
		}

		bool operator<(const SplineParameter& other) const {
			if (curveIndex < other.curveIndex) {
				return true;
			} else if (curveIndex > other.curveIndex) {
				return false;
			}
			return t < other.t;
		}
		bool operator>(const SplineParameter& other) const {
			if (curveIndex > other.curveIndex) {
				return true;
			} else if (curveIndex < other.curveIndex) {
				return false;
			}
			return t > other.t;
		}
		bool operator<=(const SplineParameter& other) const {
			if (curveIndex < other.curveIndex) {
				return true;
			} else if (curveIndex > other.curveIndex) {
				return false;
			}
			return t <= other.t;
		}
		bool operator>=(const SplineParameter& other) const {
			if (curveIndex > other.curveIndex) {
				return true;
			} else if (curveIndex < other.curveIndex) {
				return false;
			}
			return t >= other.t;
		}
	};

	using Parameter = SplineParameter;

	Parameter parameter(double param) const;
	double doubleParameter(Parameter param) const;

	/// We represent a point on a spline by a pair of its spline parameter and the coordinates of the point in the plane.
	struct SplinePoint {
		SplineParameter param;
		Point<K> point;
	};

	// === Access ===
	/// Returns a copy of the ith curve.
	Curve curve(size_t i) const;
	/// Returns the number of curves.
	size_t numCurves() const;
	/// Returns the source of the spline; that is, the first control point.
	Point<K> source() const;
	/// Returns the target of the spline; that is, the last control point.
	Point<K> target() const;
	/// Returns an iterable of all 3k+1 control points.
	const ControlsContainer& controlPoints() const;
	/// Returns the ith control point.
	Point<K> controlPoint(size_t i) const;

	/// Evaluates the spline at the spline parameter.
	Point<K> evaluate(const SplineParameter& param) const;
	/// Evaluates the curve at the spline parameter.
	Point<K> position(const SplineParameter& param) const;
	/// Evaluates the derivative at the spline parameter.
	Vector<K> derivative(const SplineParameter& param) const;
	/// Evaluates the second derivative at the spline parameter.
	Vector<K> derivative2(const SplineParameter& param) const;
	/// Computes the tangent at the spline parameter.
	Vector<K> tangent(const SplineParameter& param) const;
	/// Computes the normal at the spline parameter.
	Vector<K> normal(const SplineParameter& param) const;
	/// Computes the curvature at the spline parameter.
	Number<K> curvature(const SplineParameter& param) const;

	class CurveIterable {
	  public:
		class Iterator {
		  public:
			using iterator_category = std::random_access_iterator_tag;
			using value_type        = Curve;
			using difference_type   = std::ptrdiff_t;
			using pointer           = void;         // no persistent address
			using reference         = value_type;   // returns by value

			Iterator(const Spline* spline, size_t index)
			    : parent_(spline), index_(index) {}

			value_type operator*() const { return parent_->curve(index_); }

			// emulate operator-> by returning a proxy
			struct Proxy {
				value_type value;
				const value_type* operator->() const { return &value; }
			};
			Proxy operator->() const { return Proxy{ parent_->curve(index_) }; }

			Iterator& operator++() { ++index_; return *this; }
			Iterator& operator--() { --index_; return *this; }

			Iterator operator+(difference_type n) const { return {parent_, index_ + n}; }
			Iterator operator-(difference_type n) const { return {parent_, index_ - n}; }

			difference_type operator-(const Iterator& other) const {
				return static_cast<difference_type>(index_) -
				       static_cast<difference_type>(other.index_);
			}

			Iterator& operator+=(difference_type n) { index_ += n; return *this; }
			Iterator& operator-=(difference_type n) { index_ -= n; return *this; }

			value_type operator[](difference_type n) const {
				return parent_->curve(index_ + n);
			}

			bool operator==(const Iterator& rhs) const {
				return index_ == rhs.index_ && parent_ == rhs.parent_;
			}
			bool operator!=(const Iterator& rhs) const { return !(*this == rhs); }
			bool operator<(const Iterator& rhs)  const { return index_ < rhs.index_; }
			bool operator>(const Iterator& rhs)  const { return rhs < *this; }
			bool operator<=(const Iterator& rhs) const { return !(rhs < *this); }
			bool operator>=(const Iterator& rhs) const { return !(*this < rhs); }

		  private:
			const Spline* parent_;
			size_t index_;
		};

		CurveIterable(const Spline* parent, size_t count)
		    : parent_(parent), count_(count) {}

		Iterator begin() const { return { parent_, 0 }; }
		Iterator end()   const { return { parent_, count_ }; }
		Curve front() const { return parent_->curve(0); }
		Curve back() const { return parent_->curve(count_ - 1); }
		Curve operator[](size_t index) const { return parent_->curve(index); }

		size_t size() const { return count_; }

	  private:
		const Spline* parent_;
		size_t count_;
	};

	/// Returns an iterable of all curves.
	CurveIterable curves() const { return { this, numCurves() }; }
	CurveIterable::Iterator curves_begin() const { return curves().begin(); }
	CurveIterable::Iterator curves_end() const { return curves().end(); }

	// === Predicates ===
	/// Returns true iff the spline has no control points.
	bool empty() const;
	/// Returns true iff the spline's first and last control points are identical.
	bool closed() const;

	// === More computational operations ===
	/// Given a point, return the nearest point on the spline together with its parameter value.
	SplinePoint nearest(Point<K> point, double threshold = M_EPSILON) const;
	/// Returns the extrema on the spline: left-, bottom-, right-, top-most points on the curve.
	std::tuple<SplinePoint, SplinePoint, SplinePoint, SplinePoint> extrema() const;
	/// Returns the axis-aligned bounding box of the spline.
	Box bbox() const;
	/// Returns a copy of the spline that is the reverse of this spline.
	CubicBezierSpline reversed() const;
	/// Reverses this spline.
	void reverse();
	/// Approximates the spline with a polyline using the provided number of straight edges per curve.
	Polyline<Inexact> polyline(int nEdgesPerCurve) const;
	/// Return a transformed version of the Bézier spline.
	CubicBezierSpline transform(const CGAL::Aff_transformation_2<Inexact>& t) const;
	/// Computes the signed area of the spline.
	/// Positive for counter-clockwise curves, negative otherwise.
	/// For open splines it returns the signed area as if the curve was closed with a line segment between the endpoints.
	Number<Inexact> signedArea() const;
	/// Returns the two parts after spliting this Bézier spline at point \c p at parameter \c param.
	/// That is, the first spline of this pair starts at the source and ends at \c p.
	/// The second spline of this pair starts at \c p and ends at the target.
	/// Note that no approximation is needed for this operation, the splines match the original exactly (up to floating-point errors).
	std::pair<CubicBezierSpline, CubicBezierSpline> split(const SplineParameter& param) const;

	std::pair<CubicBezierSpline, CubicBezierSpline> split(double param) const;

	CubicBezierSpline sub(const SplineParameter& from, const SplineParameter& to) const;

	/// Outputs the parameter values (\ref SplineParameter) at which the curvature flips sign.
	template <class OutputIterator>
	void inflectionsT(OutputIterator out) const {
		std::vector<CubicBezierCurve> cs(curves_begin(), curves_end());

		for (int curveIndex = 0; curveIndex < cs.size(); ++curveIndex) {
			auto& curve = cs[curveIndex];
			std::vector<double> ts;
			curve.inflectionsT(std::back_inserter(ts));
			for (const auto& t : ts) {
				*out++ = SplineParameter{curveIndex, t};
			}

			// The vertices that connect different curves may also be inflection points.
			if (curveIndex < numCurves() - 1) {
				auto& nextCurve = cs[curveIndex+1];
				// If this is not the last curve, then the target of this curve is a curve endpoint in the interior of the spline.
				if (curve.curvature(1) * nextCurve.curvature(0) < 0) { // if curvature has different signs
					*out++ = SplineParameter{curveIndex, 1};
				}
			}
		}
	}

	/// Outputs the spline points (\ref SplinePoint) at which the curvature flips sign.
	template <class OutputIterator>
	void inflections(OutputIterator out) const {
		std::vector<CubicBezierCurve> cs(curves_begin(), curves_end());

		for (int curveIndex = 0; curveIndex < cs.size(); ++curveIndex) {
			auto& curve = cs[curveIndex];
			std::vector<Curve::CurvePoint> pts;
			curve.inflections(std::back_inserter(pts));
			for (const auto& pt : pts) {
				*out++ = SplinePoint{SplineParameter{curveIndex, pt.t}, pt.point};
			}

			// The vertices that connect different curves may also be inflection points.
			if (curveIndex < numCurves() - 1) {
				auto& nextCurve = cs[curveIndex + 1];
				// If this is not the last curve, then the target of this curve is a curve endpoint in the interior of the spline.
				if (curve.curvature(1) * nextCurve.curvature(0) < 0) { // if curvature has different signs
					*out++ = SplinePoint{SplineParameter{curveIndex, 1}, curve.target()};
				}
			}
		}
	}

	/// Outputs the spline parameter values at which the Bézier spline intersects the given line.
	template <class OutputIterator>
	void intersectionsT(const Line<K>& line, OutputIterator out) const {
		int curveIndex = 0;
		for (const auto& curve : curves()) {
			std::vector<double> ts;
			curve.intersectionsT(line, std::back_inserter(ts));
			for (double t : ts) {
				*out++ = SplineParameter{curveIndex, t};
			}
			++curveIndex;
		}
	}

	/// Outputs the spline points (\ref SplinePoint) at which the Bézier spline intersects the given line.
	template <class OutputIterator>
	void intersections(const Line<K>& line, OutputIterator out) const {
		std::vector<SplineParameter> intersParams;
		intersectionsT(line, std::back_inserter(intersParams));
		for (const auto& param : intersParams) {
			*out++ = SplinePoint{param, evaluate(param)};
		}
	}

	/// Outputs the spline points (\ref SplinePoint) at which the Bézier spline intersects the given line segment.
	template <class OutputIterator>
	void intersections(const Segment<K>& segment, OutputIterator out) const {
		int curveIndex = 0;
		for (const auto& curve : curves()) {
			std::vector<CubicBezierCurve::CurvePoint> curvePoints;
			curve.intersections(segment, std::back_inserter(curvePoints));
			for (const auto& cp : curvePoints) {
				*out++ = SplinePoint{.param=SplineParameter{curveIndex, cp.t}, .point=cp.point};
				++curveIndex;
			}
		}
	}

	/// Outputs the spline points (\ref SplinePoint) at which the Bézier spline intersects the given ray.
	template <class OutputIterator>
	void intersections(const Ray<K>& ray, OutputIterator out) const {
		int curveIndex = 0;
		for (const auto& curve : curves()) {
			std::vector<CubicBezierCurve::CurvePoint> curvePoints;
			curve.intersections(ray, std::back_inserter(curvePoints));
			for (const auto& cp : curvePoints) {
				*out++ = SplinePoint{.param=SplineParameter{curveIndex, cp.t}, .point=cp.point};
				++curveIndex;
			}
		}
	}

	bool intersects(const CubicBezierCurve& other, double threshold = M_EPSILON) const;
	bool intersects(const CubicBezierSpline& other, double threshold = M_EPSILON) const;
	template <class OutputIterator>
	void intersections(const CubicBezierCurve& other, OutputIterator out, double threshold = M_EPSILON) const {
		detail::intersectionsRecursive(*this, other, out, 0, 1, 0, 1, threshold);
	}
	template <class OutputIterator>
	void intersections(const CubicBezierSpline& other, OutputIterator out, double threshold = M_EPSILON) const {
		detail::intersectionsRecursive(*this, other, out, 0, 1, 0, 1, threshold);
	}
	bool selfIntersects(double threshold = M_EPSILON) const;

	CGAL::Bounded_side boundedSide(const Point<Inexact>& p) const;
};
}