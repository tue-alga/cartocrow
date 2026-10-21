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

#include "cubic_bezier.h"

#include <CGAL/Bbox_2.h>

#include <ranges>

namespace cartocrow {
CubicBezierCurve::CubicBezierCurve(Point<K> source, Point<K> control1, Point<K> control2,
                                   Point<K> target)
    : m_p0(std::move(source)), m_p1(std::move(control1)), m_p2(std::move(control2)),
      m_p3(std::move(target)) {
	m_v0 = m_p0 - CGAL::ORIGIN;
	m_v1 = m_p1 - CGAL::ORIGIN;
	m_v2 = m_p2 - CGAL::ORIGIN;
	m_v3 = m_p3 - CGAL::ORIGIN;
};

CubicBezierCurve::CubicBezierCurve(Point<K> source, Point<K> control, Point<K> target)
    : CubicBezierCurve(source, source + (control - source) * 2.0 / 3.0,
                       target + (control - target) * 2.0 / 3.0, target) {}

CubicBezierCurve::CubicBezierCurve(Point<K> source, Point<K> target)
    : CubicBezierCurve(source, source + (target - source) * 1.0 / 3.0,
                       source + (target - source) * 2.0 / 3.0, target) {}

CubicBezierCurve::CubicBezierCurve(Segment<K> seg) : CubicBezierCurve(seg.source(), seg.target()) {}

Point<Inexact> CubicBezierCurve::source() const {
	return m_p0;
}

Point<Inexact> CubicBezierCurve::sourceControl() const {
	return m_p1;
}

Point<Inexact> CubicBezierCurve::targetControl() const {
	return m_p2;
}

Point<Inexact> CubicBezierCurve::target() const {
	return m_p3;
}

Point<Inexact> CubicBezierCurve::control(int i) const {
	assert(i >= 0 && i <= 3);
	switch(i) {
		case 0: return m_p0;
	    case 1: return m_p1;
	    case 2: return m_p2;
	    case 3: return m_p3;
	    default: throw std::runtime_error("A cubic Bézier curve only has control points 0, 1, 2, and 3.");
	}
}

Point<Inexact> CubicBezierCurve::evaluate(const Number<Inexact>& t) const {
	auto t_ = 1 - t;
	auto a = t_ * t_ * t_;
	auto b = 3 * t * t_ * t_;
	auto c = 3 * t * t * t_;
	auto d = t * t * t;

	return CGAL::ORIGIN + a * m_v0 + b * m_v1 + c * m_v2 + d * m_v3;
}

Point<Inexact> CubicBezierCurve::position(const Number<Inexact>& t) const {
	return evaluate(t);
}

Vector<Inexact> CubicBezierCurve::derivative(const Number<Inexact>& t) const {
	auto v1 = m_p1 - m_p0;
	auto v2 = m_p2 - m_p1;
	auto v3 = m_p3 - m_p2;

	auto t_ = 1 - t;
	auto a = 3 * t_ * t_;
	auto b = 6 * t_ * t;
	auto c = 3 * t * t;

	return a * v1 + b * v2 + c * v3;
}

Vector<Inexact> CubicBezierCurve::derivative2(const Number<Inexact>& t) const {
	auto v1 = m_v2 - 2 * m_v1 + m_v0;
	auto v2 = m_v3 - 2 * m_v2 + m_v1;

	auto t_ = 1 - t;

	return 6 * t_ * v1 + 6 * t * v2;
}

Vector<Inexact> CubicBezierCurve::tangent(const Number<Inexact>& t) const {
	return derivative(t);
}

Vector<Inexact> CubicBezierCurve::normal(const Number<Inexact>& t) const {
	auto d = derivative(t);
	return {-d.y(), d.x()};
}

Number<Inexact> CubicBezierCurve::signedArea() const {
	const auto& x0 = m_p0.x();
	const auto& x1 = m_p1.x();
	const auto& x2 = m_p2.x();
	const auto& x3 = m_p3.x();
	const auto& y0 = m_p0.y();
	const auto& y1 = m_p1.y();
	const auto& y2 = m_p2.y();
	const auto& y3 = m_p3.y();

	return -( x0 * (      - 2*y1 -   y2 + 3*y3)
	        + x1 * ( 2*y0        -   y2 -   y3)
	        + x2 * (   y0 +   y1        - 2*y3)
	        + x3 * (-3*y0 +   y1 + 2*y2       )
	            ) * 3 / 20;
}

CubicBezierCurve CubicBezierCurve::reversed() const {
	return {m_p3, m_p2, m_p1, m_p0};
}

void CubicBezierCurve::reverse() {
	std::swap(m_p0, m_p3);
	std::swap(m_p1, m_p2);
	std::swap(m_v0, m_v3);
	std::swap(m_v1, m_v2);
}

std::pair<CubicBezierCurve, CubicBezierCurve> 
CubicBezierCurve::split(const Number<Inexact>& t) const {
	auto t_ = 1 - t;

	auto x0 = t_ * m_v0 + t * m_v1;
	auto x1 = t_ * m_v1 + t * m_v2;
	auto x2 = t_ * m_v2 + t * m_v3;

	auto y0 = t_ * x0 + t * x1;
	auto y1 = t_ * x1 + t * x2;

	auto z = t_ * y0 + t * y1;

	CubicBezierCurve b1(m_p0, CGAL::ORIGIN + x0, CGAL::ORIGIN + y0, CGAL::ORIGIN + z);
	CubicBezierCurve b2(CGAL::ORIGIN + z, CGAL::ORIGIN + y1, CGAL::ORIGIN + x2, m_p3);

	return {b1, b2};
}

CubicBezierCurve 
CubicBezierCurve::sub(const Number<Inexact>& t1, const Number<Inexact>& t2) const {
	auto [_, tail] = split(t1);
	double t2_ = (t2 - t1) / (1 - t1);
	return tail.split(t2_).first;
}

Polyline<Inexact> CubicBezierCurve::polyline(int nEdges) const {
	Polyline<Inexact> pl;
	if (nEdges < 1) return pl;

	samplePoints(nEdges + 1, std::back_inserter(pl));

	return pl;
}

Number<Inexact> CubicBezierCurve::curvature(Number<Inexact> t) const {
	auto d = derivative(t);
	auto dd = derivative2(t);

	auto num = d.x() * dd.y() - dd.x() * d.y();
	auto den = pow(d.x() * d.x() + d.y() * d.y(), 3.0 / 2.0);
	if (den == 0) {
		return std::numeric_limits<double>::infinity();
	}
	return num / den;
}

CubicBezierCurve CubicBezierCurve::transform(const CGAL::Aff_transformation_2<Inexact>& t) const {
	return {m_p0.transform(t), m_p1.transform(t), m_p2.transform(t), m_p3.transform(t)};
}

CubicBezierCurve::Parameter CubicBezierCurve::parameter(double param) const {
	return param;
}

double CubicBezierCurve::doubleParameter(CubicBezierCurve::Parameter param) const {
	return param;
}

CubicBezierCurve::CurvePoint CubicBezierCurve::nearest(Point<K> point, double threshold) const {
	int nPoints = 10;

	int closest = -1;
	double minDist2 = std::numeric_limits<double>::infinity();

	std::vector<Point<K>> points;
	samplePoints(nPoints, std::back_inserter(points));

	for (int i = 0; i < nPoints; ++i) {
		auto dist2 = CGAL::squared_distance(points[i], point);
		if (dist2 < minDist2) {
			minDist2 = dist2;
			closest = i;
		}
	}

	// now do binary search
	auto indexToT = [nPoints](int index) {
		double step = 1.0 / (nPoints - 1);
		return index * step;
	};
	double t1 = indexToT(std::max(0, closest - 1));
	double t2 = indexToT(std::min(nPoints - 1, closest + 1));
	double closestT = indexToT(closest);
	auto closestP = points[closest];

	while (t2 - t1 > threshold) {
		double mt = (t1 + t2) / 2;
		double s1 = (t1 + mt) / 2;
		double s2 = (mt + t2) / 2;
		auto s1P = evaluate(s1);
		auto s2P = evaluate(s2);
		auto mtP = evaluate(mt);
		double s1D2 = CGAL::squared_distance(s1P, point);
		double s2D2 = CGAL::squared_distance(s2P, point);
		double mtD2 = CGAL::squared_distance(mtP, point);
		double minD = std::min(std::min(s1D2, s2D2), std::min(minDist2, mtD2));
		if (minD == s1D2) {
			minDist2 = minD;
			closestT = s1;
			closestP = s1P;
			t2 = mt;
		} else if (minD == s2D2) {
			minDist2 = minD;
			closestT = s2;
			closestP = s2P;
			t1 = mt;
		} else if (minD == mtD2) {
			minDist2 = minD;
			t1 = s1;
			t2 = s2;
			closestT = mt;
			closestP = mtP;
		} else {
			assert(minD == minDist2);
			assert(closestT == t1 || closestT == t2);
			if (closestT == t1) {
				t2 = s1;
			} else {
				t1 = s2;
			}
		}
	}

	return {closestT, closestP};
}

std::tuple<CubicBezierCurve::CurvePoint, CubicBezierCurve::CurvePoint, CubicBezierCurve::CurvePoint, CubicBezierCurve::CurvePoint>
CubicBezierCurve::extrema() const {
	auto a = 3 * (-m_v0 + 3 * m_v1 - 3 * m_v2 + m_v3);
	auto b = 6 * (m_v0 - 2 * m_v1 + m_v2);
	auto c = 3 * (m_v1 - m_v0);

	double xMinT = 0;
	double yMinT = 0;
	double xMaxT = 0;
	double yMaxT = 0;
	Point<K> xMinP = m_p0;
	Point<K> yMinP = m_p0;
	Point<K> xMaxP = m_p0;
	Point<K> yMaxP = m_p0;
	if (m_p3.x() < xMinP.x()) {
		xMinT = 1;
		xMinP = m_p3;
	}
	if (m_p3.x() > xMaxP.x()) {
		xMaxT = 1;
		xMaxP = m_p3;
	}
	if (m_p3.y() < yMinP.y()) {
		yMinT = 1;
		yMinP = m_p3;
	}
	if (m_p3.y() > yMaxP.y()) {
		yMaxT = 1;
		yMaxP = m_p3;
	}

	auto checkExtrema = [this](double A, double B, double C, auto coordSelector, Point<K>& minVal,
	                           Point<K>& maxVal, double& minValT, double& maxValT) {
		double D = B * B - 4 * A * C;
		if (D < 0)
			return;
		double sqrtD = std::sqrt(D);
		double denom = 2 * A;
		if (denom == 0)
			return;

		for (double t : {(-B - sqrtD) / denom, (-B + sqrtD) / denom}) {
			if (t >= 0.0 && t <= 1.0) {
				auto p = evaluate(t);
				double val = coordSelector(p);
				if (val < coordSelector(minVal)) {
					minVal = p;
					minValT = t;
				}
				if (val > coordSelector(maxVal)) {
					maxVal = p;
					maxValT = t;
				}
			}
		}
	};

	checkExtrema(a.x(), b.x(), c.x(), [](auto& p) { return p.x(); }, xMinP, xMaxP, xMinT, xMaxT);
	checkExtrema(a.y(), b.y(), c.y(), [](auto& p) { return p.y(); }, yMinP, yMaxP, yMinT, yMaxT);

	return {{xMinT, xMinP}, {yMinT, yMinP}, {xMaxT, xMaxP}, {yMaxT, yMaxP}};
}

Box CubicBezierCurve::bbox() const {
	auto [left, bottom, right, top] = extrema();
	return {left.point.x(), bottom.point.y(), right.point.x(), top.point.y()};
}

std::tuple<Vector<Inexact>, Vector<Inexact>, Vector<Inexact>, Vector<Inexact>>
CubicBezierCurve::coefficients() const {
	auto c0 = m_v3 - m_v0 + 3 * (m_v1 - m_v2);
	auto c1 = 3 * (m_v0 - 2 * m_v1 + m_v2);
	auto c2 = 3 * (m_v1 - m_v0);
	auto c3 = m_v0;
	return {c0, c1, c2, c3};
}

bool CubicBezierCurve::intersects(const CubicBezierCurve& other, double threshold) const {
	return detail::intersectsRecursive(*this, other, threshold);
}

bool CubicBezierCurve::intersects(const CubicBezierSpline& other, double threshold) const {
	return detail::intersectsRecursive(*this, other, threshold);
}

bool CubicBezierCurve::selfIntersects(double threshold) const {
	std::vector<CubicBezierCurve> curves;
	monotoneParts(std::back_inserter(curves));

	for (int i = 0; i < curves.size(); ++i) {
		for (int j = i + 2; j < curves.size(); ++j) { // do not check consecutive parts
			if (detail::intersectsRecursive(curves[i], curves[j], threshold))
				return true;
		}
	}

	return false;
}

CubicBezierSpline::CubicBezierSpline() {};

CubicBezierSpline::CubicBezierSpline(Segment<K> segment) {
	appendCurve(segment);
}

CubicBezierSpline::CubicBezierSpline(CubicBezierCurve curve)
    : m_c({curve.source(), curve.sourceControl(), curve.targetControl(), curve.target()}) {};

CubicBezierSpline::CubicBezierSpline(Polyline<K> polyline) {
	for (int i = 0; i < polyline.num_edges(); ++i) {
		appendCurve(polyline.vertex(i), polyline.vertex(i + 1));
	}
}

void CubicBezierSpline::appendCurve(const Curve& curve) {
	for (int i = 0; i < 4; ++i) {
		if (i == 0 && !m_c.empty()) {
			assert(m_c.back() == curve.source());
			continue;
		}
		m_c.push_back(curve.control(i));
	}
}

void CubicBezierSpline::appendCurve(Point<K> source, Point<K> control1, Point<K> control2,
                                    Point<K> target) {
	if (m_c.empty()) {
		m_c.push_back(std::move(source));
	} else {
		assert(m_c.back() == source);
	}
	m_c.push_back(std::move(control1));
	m_c.push_back(std::move(control2));
	m_c.push_back(std::move(target));
}

void CubicBezierSpline::appendCurve(Point<K> source, Point<K> control, Point<K> target) {
	if (m_c.empty()) {
		m_c.push_back(source);
	} else {
		assert(m_c.back() == source);
	}
	m_c.push_back(source + (control - source) * 2.0 / 3.0);
	m_c.push_back(target + (control - target) * 2.0 / 3.0);
	m_c.push_back(target);
}

void CubicBezierSpline::appendCurve(Point<K> source, Point<K> target) {
	if (m_c.empty()) {
		m_c.push_back(source);
	} else {
		assert(m_c.back() == source);
	}
	m_c.push_back(source + (target - source) * 1.0 / 3.0);
	m_c.push_back(source + (target - source) * 2.0 / 3.0);
	m_c.push_back(target);
}

void CubicBezierSpline::appendSpline(const CubicBezierSpline& spline) {
	for (int i = 0; i < spline.controlPoints().size(); ++i) {
		if (i == 0 && !m_c.empty()) {
			assert(m_c.back() == spline.source());
			continue;
		}
		m_c.push_back(spline.controlPoint(i));
	}
}

CubicBezierCurve CubicBezierSpline::curve(size_t i) const {
	return {m_c[3 * i], m_c[3 * i + 1], m_c[3 * i + 2], m_c[3 * i + 3]};
}

const std::vector<Point<Inexact>>& CubicBezierSpline::controlPoints() const {
	return m_c;
};

Point<Inexact> CubicBezierSpline::controlPoint(size_t i) const {
	return m_c[i];
}

Point<Inexact> CubicBezierSpline::source() const {
	return m_c.front();
}

Point<Inexact> CubicBezierSpline::target() const {
	return m_c.back();
}

size_t CubicBezierSpline::numCurves() const {
	if (m_c.empty())
		return 0;
	return (m_c.size() - 1) / 3;
}

bool CubicBezierSpline::empty() const {
	return m_c.empty();
}

bool CubicBezierSpline::closed() const {
	return m_c.front() == m_c.back();
}

Point<Inexact> CubicBezierSpline::evaluate(const SplineParameter& param) const {
	auto c = curve(param.curveIndex);
	return c.evaluate(param.t);
}

Point<Inexact> CubicBezierSpline::position(const SplineParameter& param) const {
	auto c = curve(param.curveIndex);
	return c.position(param.t);
}

Vector<Inexact> CubicBezierSpline::derivative(const SplineParameter& param) const {
	auto c = curve(param.curveIndex);
	return c.derivative(param.t);
}

Vector<Inexact> CubicBezierSpline::derivative2(const SplineParameter& param) const {
	auto c = curve(param.curveIndex);
	return c.derivative2(param.t);
}

Vector<Inexact> CubicBezierSpline::tangent(const SplineParameter& param) const {
	auto c = curve(param.curveIndex);
	return c.tangent(param.t);
}

Vector<Inexact> CubicBezierSpline::normal(const SplineParameter& param) const {
	auto c = curve(param.curveIndex);
	return c.normal(param.t);
}

Number<Inexact> CubicBezierSpline::curvature(const SplineParameter& param) const {
	auto c = curve(param.curveIndex);
	return c.curvature(param.t);
}

CubicBezierSpline::SplinePoint
CubicBezierSpline::nearest(Point<Inexact> point, double threshold) const {
	double minDist2 = std::numeric_limits<double>::infinity();
	int closestIndex = -1;
	Curve::CurvePoint closest;

	int curveIndex = 0;
	for (const auto& c : curves()) {
		auto n = c.nearest(point, threshold);
		auto dist2 = CGAL::squared_distance(point, n.point);
		if (dist2 < minDist2) {
			minDist2 = dist2;
			closestIndex = curveIndex;
			closest = n;
		}
		++curveIndex;
	}

	return SplinePoint{SplineParameter{closestIndex, closest.t}, closest.point};
}

std::tuple<CubicBezierSpline::SplinePoint, CubicBezierSpline::SplinePoint,
           CubicBezierSpline::SplinePoint, CubicBezierSpline::SplinePoint>
CubicBezierSpline::extrema() const {
	std::optional<SplinePoint> l, b, r, t;
	int curveIndex = 0;
	for (const auto& c : curves()) {
		auto [l_, b_, r_, t_] = c.extrema();
		if (!l.has_value() || l_.point.x() < l->point.x()) {
			l = SplinePoint{.param = SplineParameter{.curveIndex = curveIndex, .t = l_.t},
			                .point = l_.point};
		}
		if (!r.has_value() || r_.point.x() > r->point.x()) {
			r = SplinePoint{.param = SplineParameter{.curveIndex = curveIndex, .t = r_.t},
			                .point = r_.point};
		}
		if (!b.has_value() || b_.point.y() < b->point.y()) {
			b = SplinePoint{.param = SplineParameter{.curveIndex = curveIndex, .t = b_.t},
			                .point = b_.point};
		}
		if (!t.has_value() || t_.point.y() > t->point.y()) {
			t = SplinePoint{.param = SplineParameter{.curveIndex = curveIndex, .t = t_.t},
			                .point = t_.point};
		}
		++curveIndex;
	}
	return {*l, *b, *r, *t};
}

Box CubicBezierSpline::bbox() const {
	auto curves_view = curves();
	std::vector<CubicBezierCurve> temp(curves_view.begin(), curves_view.end());
	return CGAL::bbox_2(temp.begin(), temp.end());
}

void CubicBezierSpline::reverse() {
	std::reverse(m_c.begin(), m_c.end());
}

CubicBezierSpline CubicBezierSpline::reversed() const {
	CubicBezierSpline rev;
	std::copy(m_c.rbegin(), m_c.rend(), std::back_inserter(rev.m_c));
	return rev;
}

Number<Inexact> CubicBezierSpline::signedArea() const {
	if (empty())
		return 0;
	if (numCurves() == 1)
		return curves().front().signedArea();

	Polygon<Inexact> p;
	p.push_back(curves().front().source());

	Number<Inexact> a = 0;
	for (const auto& c : curves()) {
		p.push_back(c.target());
		a += c.signedArea();
	}
	a += p.area();

	return a;
}

std::pair<CubicBezierSpline, CubicBezierSpline>
CubicBezierSpline::split(const SplineParameter& param) const {
	// Split the curve that is affected.
	std::optional<CubicBezierCurve> c1;
	std::optional<CubicBezierCurve> c2;

	auto c = curve(param.curveIndex);
	if (param.t < M_EPSILON) {
		c2 = c;
	} else if (param.t > 1 - M_EPSILON) {
		c1 = c;
	} else {
		std::tie(c1, c2) = c.split(param.t);
	}

	// Control points of other curves stay the same, copy them over appropriately.
	CubicBezierSpline spline1;
	for (int i = 0; i < param.curveIndex; ++i) {
		if (spline1.empty())
			spline1.m_c.push_back(m_c[3 * i]);
		spline1.m_c.push_back(m_c[3 * i + 1]);
		spline1.m_c.push_back(m_c[3 * i + 2]);
		spline1.m_c.push_back(m_c[3 * i + 3]);
	}
	if (c1.has_value()) {
		spline1.appendCurve(*c1);
	}

	CubicBezierSpline spline2;
	if (c2.has_value()) {
		spline2.appendCurve(*c2);
	}
	for (int i = param.curveIndex + 1; i < numCurves(); ++i) {
		if (spline2.empty())
			spline2.m_c.push_back(m_c[3 * i]);
		spline2.m_c.push_back(m_c[3 * i + 1]);
		spline2.m_c.push_back(m_c[3 * i + 2]);
		spline2.m_c.push_back(m_c[3 * i + 3]);
	}

	return {spline1, spline2};
}

CubicBezierSpline CubicBezierSpline::sub(const CubicBezierSpline::SplineParameter& from,
                                         const CubicBezierSpline::SplineParameter& to) const {
	if (from.curveIndex == to.curveIndex) {
		return curve(from.curveIndex).sub(from.t, to.t);
	}
	// This can be optimized by not constructing curves but directly setting control points.
	CubicBezierSpline s;
	s.appendCurve(curve(from.curveIndex).sub(from.t, 1.0));
	for (int i = from.curveIndex + 1; i < to.curveIndex; ++i) {
		s.appendCurve(curve(i));
	}
	s.appendCurve(curve(to.curveIndex).sub(0.0, to.t));
	return s;
}

CubicBezierSpline::Parameter CubicBezierSpline::parameter(double param) const {
	double t = param * numCurves();
	int curveIndex = static_cast<int>(t);
	double frac = t - curveIndex;
	return {curveIndex, frac};
}

double CubicBezierSpline::doubleParameter(CubicBezierSpline::Parameter param) const {
	return (param.curveIndex + param.t) / static_cast<double>(numCurves());
}

std::pair<CubicBezierSpline, CubicBezierSpline> CubicBezierSpline::split(double param) const {
	return split(parameter(param));
}

Polyline<Inexact> CubicBezierSpline::polyline(int nEdgesPerCurve) const {
	Polyline<Inexact> pl;
	if (nEdgesPerCurve < 1)
		return pl;

	pl.push_back(curves().front().source());
	for (const auto& curve : curves()) {
		int nPoints = nEdgesPerCurve + 1;

		double step = 1.0 / (nPoints - 1);
		for (int i = 1; i < nPoints; ++i) {
			double t = i * step;
			pl.push_back(curve.evaluate(t));
		}
	}

	return pl;
}

CubicBezierSpline CubicBezierSpline::transform(const CGAL::Aff_transformation_2<Inexact>& t) const {
	CubicBezierSpline transformed;
	for (const auto& cp : m_c) {
		transformed.m_c.push_back(cp.transform(t));
	}
	return transformed;
}

bool CubicBezierSpline::intersects(const CubicBezierCurve& other, double threshold) const {
	return detail::intersectsRecursive(*this, other, threshold);
}

bool CubicBezierSpline::intersects(const CubicBezierSpline& other, double threshold) const {
	return detail::intersectsRecursive(*this, other, threshold);
}

bool CubicBezierSpline::selfIntersects(double threshold) const {
	std::vector<CubicBezierCurve> parts;
	for (const auto& curve : curves()) {
		curve.monotoneParts(std::back_inserter(parts));
	}

	for (int i = 0; i < parts.size(); ++i) {
		for (int j = i + 2; j < parts.size(); ++j) { // do not check consecutive parts
			if (detail::intersectsRecursive(parts[i], parts[j], threshold)) return true;
		}
	}

	return false;
}

// Note that the current implementation is not robust.
CGAL::Bounded_side CubicBezierSpline::boundedSide(const Point<Inexact>& p) const {
	Ray<Inexact> r(p, Vector<Inexact>(1, 0));

	std::vector<SplinePoint> inters;
	intersections(r, std::back_inserter(inters));
	if (inters.size() % 2 == 0) {
		return CGAL::ON_UNBOUNDED_SIDE;
	} else {
		return CGAL::ON_BOUNDED_SIDE;
	}
}
}
