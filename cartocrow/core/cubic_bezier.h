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
	/// Construct a cubic Bézier curve from its two endpoints and two control points.
	CubicBezierCurve(Point<K> source, Point<K> control1, Point<K> control2, Point<K> target);

	/// Construct a cubic Bézier curve from two endpoints and one control point (i.e. from a quadratic Bézier curve).
	CubicBezierCurve(Point<K> source, Point<K> control, Point<K> target);

	/// Construct a cubic Bézier curve from two endpoints.
	CubicBezierCurve(Point<K> source, Point<K> target);

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

	/// Returns the two parts after spliting this Bézier curve at point \c p at time \c t.
	/// That is, the first curve of this pair starts at the source and ends at \c p.
	/// The second curve of this pair \c p at p and ends at target.
	/// Note that no approximation is needed for this operation, the curves match the original exactly (up to floating-point errors).
	std::pair<CubicBezierCurve, CubicBezierCurve> split(const Number<K>& t) const;

	/// Returns a naive approximation of this Bézier curve by a polyline with nEdges.
	/// The polyline starts at the source and ends at the target of the curve.
	/// All vertices of the polyline lie on the Bézier curve and their parameter values (not the points) are equidistant
	Polyline<K> polyline(int nEdges) const;

	/// Return a transformed version of the Bézier curve.
	CubicBezierCurve transform(const CGAL::Aff_transformation_2<Inexact> &t) const;

	struct CurvePoint {
		Number<K> t;
		Point<K> point;
	};

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
			*out++ = CurvePoint(t, evaluate(t));
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
				*out++ = CurvePoint(t, point);
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
			if (s >= 0 && s <= 1) {
				*out++ = CurvePoint(t, point);
			}
		}
	}

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
			*out++ = CurvePoint(t, evaluate(t));
		}
	}

	/// Outputs approximations of the intersection points (Point<Inexact>) with another cubic Bézier curve
	template <class OutputIterator>
	void intersections(const CubicBezierCurve& other, OutputIterator out) {
		using Nt_traits = CGAL::CORE_algebraic_number_traits;
		using NT = Nt_traits::Rational;
		using Rational = Nt_traits::Rational;
		using Algebraic = Nt_traits::Algebraic;
		using Rat_kernel = CGAL::Cartesian<Rational>;
		using Alg_kernel = CGAL::Cartesian<Algebraic>;
		using Rat_point = Rat_kernel::Point_2;
		using Traits =
		    CGAL::Arr_Bezier_curve_traits_2<Rat_kernel, Alg_kernel, Nt_traits>;
		using Bezier_curve = Traits::Curve_2;

		auto rp = [](const Point<Inexact>& p) {
			return Rat_point(p.x(), p.y());
		};
		std::vector<Rat_point> controls1({rp(m_p0), rp(m_p1), rp(m_p2), rp(m_p3)});
		Bezier_curve cgalCurve1(controls1.begin(), controls1.end());

		std::vector<Rat_point> controls2({rp(other.m_p0), rp(other.m_p1), rp(other.m_p2), rp(other.m_p3)});
		Bezier_curve cgalCurve2(controls2.begin(), controls2.end());

		std::vector<Bezier_curve> curves;
		std::vector<Traits::Point_2> inters;
		Traits traits;

		// The following does not return any intersections for some reason
//		CGAL::compute_intersection_points(curves.begin(), curves.end(), std::back_inserter(inters), false, traits);

		// So just create an arrangement
		CGAL::Arrangement_2<Traits> arr;
		// Insertion of curves crashes sometimes.
		// todo: report to cgal
		CGAL::insert(arr, cgalCurve1);
		CGAL::insert(arr, cgalCurve2);
		for (const auto& vh : arr.vertex_handles()) {
			// Todo: properly determine what is an intersection.
			// In particular, self-intersections of curves are also reported currently
			if (!vh->is_isolated() && vh->degree() > 2)
				inters.push_back(vh->point());
		}
		for (const auto& pt : inters) {
			auto [x, y] = pt.approximate();
			*out++ = Point<Inexact>(x, y);
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
	// === Creation ===
	/// Create an empty spline.
	CubicBezierSpline();

	/// Create a spline from a sequence of 3k+1 control points (Point<Inexact>).
	template <class InputIterator>
	CubicBezierSpline(InputIterator begin, InputIterator end) : m_c(begin, end)
		{ assert(m_c.size() == 0 || (m_c.size() - 1) % 3 == 0);}

	/// Append a cubic Bézier curve.
	void appendCurve(const Curve& curve);
	/// Append a cubic Bézier curve from its two endpoints and two control points.
	void appendCurve(Point<K> source, Point<K> control1, Point<K> control2, Point<K> target);
	/// Append a cubic Bézier curve from two endpoints and one control point (i.e. from a quadratic Bézier curve).
	void appendCurve(Point<K> source, Point<K> control, Point<K> target);
	/// Append a cubic Bézier curve from two endpoints.
	void appendCurve(Point<K> source, Point<K> target);

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
	/// Returns the axis-aligned bounding box of the spline.
	Box bbox() const;
	/// Returns a copy of the spline that is the reverse of this spline.
	CubicBezierSpline reversed() const;
	/// Reverses this spline.
	void reverse();
	/// Approximates the spline with a polyline using the provided number of straight edges per curve.
	Polyline<Inexact> polyline(int nEdgesPerCurve) const;
	/// Computes the signed area of the spline.
	/// Positive for counter-clockwise curves, negative otherwise.
	/// For open splines it returns the signed area as if the curve was closed with a line segment between the endpoints.
	Number<Inexact> signedArea() const;
};
}