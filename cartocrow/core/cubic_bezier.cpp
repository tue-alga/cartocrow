#include "cubic_bezier.h"

#include <CGAL/Bbox_2.h>

#include <ranges>

namespace cartocrow {
CubicBezierCurve::CubicBezierCurve(Point<K> source, Point<K> control1, Point<K> control2, Point<K> target) : m_p0(std::move(source)), m_p1(std::move(control1)), m_p2(std::move(control2)),
      m_p3(std::move(target)) {
	m_v0 = m_p0 - CGAL::ORIGIN;
	m_v1 = m_p1 - CGAL::ORIGIN;
	m_v2 = m_p2 - CGAL::ORIGIN;
	m_v3 = m_p3 - CGAL::ORIGIN;
};

CubicBezierCurve::CubicBezierCurve(Point<K> source, Point<K> control, Point<K> target) :
	CubicBezierCurve(source,
				     source + (control - source) * 2.0 / 3.0,
                     target + (control - target) * 2.0 / 3.0,
                     target) {}

CubicBezierCurve::CubicBezierCurve(Point<K> source, Point<K> target) :
	CubicBezierCurve(source,
                     source + (target - source) * 1.0 / 3.0,
					 source + (target - source) * 2.0 / 3.0,
                     target) {}

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
	auto c = 3 * t  * t;

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
	return {m_p3, m_p2, m_p1, m_p0 };
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

Polyline<Inexact> CubicBezierCurve::polyline(int nEdges) const {
	Polyline<Inexact> pl;
	if (nEdges < 1) return pl;

	int nPoints = nEdges + 1;

	double step = 1.0 / nEdges;
	for (int i = 0; i < nPoints; ++i) {
		double t = i * step;
		pl.push_back(evaluate(t));
	}

	return pl;
}

Number<Inexact> CubicBezierCurve::curvature(Number<Inexact> t) const {
	auto d = derivative(t);
	auto dd = derivative2(t);

	auto num = d.x() * dd.y() - dd.x() * d.y();
	auto den = pow(d.x() * d.x() + d.y() * d.y(), 3.0/2.0);
	if (den == 0) {
		return std::numeric_limits<double>::infinity();
	}
	return num / den;
}

CubicBezierCurve CubicBezierCurve::transform(const CGAL::Aff_transformation_2<Inexact>& t) const {
	return { m_p0.transform(t), m_p1.transform(t), m_p2.transform(t), m_p3.transform(t) };
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

	auto checkExtrema = [this](double A, double B, double C, auto coordSelector,
	                        Point<K>& minVal, Point<K>& maxVal, double& minValT, double& maxValT) {
		double D = B*B - 4*A*C;
		if (D < 0) return;
		double sqrtD = std::sqrt(D);
		double denom = 2*A;
		if (denom == 0) return;

		for (double t : { (-B - sqrtD)/denom, (-B + sqrtD)/denom }) {
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

	checkExtrema(a.x(), b.x(), c.x(), [](auto& p){ return p.x(); }, xMinP, xMaxP, xMinT, xMaxT);
	checkExtrema(a.y(), b.y(), c.y(), [](auto& p){ return p.y(); }, yMinP, yMaxP, yMinT, yMaxT);

	return {{xMinT, xMinP}, {yMinT, yMinP}, {xMaxT, xMaxP}, {yMaxT, yMaxP}};
}

Box CubicBezierCurve::bbox() const {
	auto [left, bottom, right, top] = extrema();
	return {left.point.x(), bottom.point.y(), right.point.x(), top.point.y()};
}

std::tuple<Vector<Inexact>, Vector<Inexact>, Vector<Inexact>, Vector<Inexact>> CubicBezierCurve::coefficients() const {
	auto c0 = m_v3 - m_v0 + 3 * (m_v1 - m_v2);
	auto c1 = 3 * (m_v0 - 2 * m_v1 + m_v2);
	auto c2 = 3 * (m_v1 - m_v0);
	auto c3 = m_v0;
	return {c0, c1, c2, c3};
}

CubicBezierSpline::CubicBezierSpline() {};

void CubicBezierSpline::appendCurve(const Curve& curve) {
	for (int i = 0; i < 4; ++i) {
		if (i == 0 && !m_c.empty()) {
			assert(m_c.back() == curve.source());
		}
		m_c.push_back(curve.control(i));
	}
}

void CubicBezierSpline::appendCurve(Point<K> source, Point<K> control1, Point<K> control2, Point<K> target) {
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
	if (m_c.empty()) return 0;
	return (m_c.size() - 1) / 3;
}

bool CubicBezierSpline::empty() const {
	return m_c.empty();
}

bool CubicBezierSpline::closed() const {
	return m_c.front() == m_c.back();
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
	if (empty()) return 0;
	if (numCurves() == 1) return curves().front().signedArea();

	Polygon<Inexact> p;
	p.push_back(curves().front().source());

	Number<Inexact> a = 0;
	for (const auto& c : curves()) {
		p.push_back(c.target());
		a += c.signedArea();
	}
	a -= p.area();

	return a;
}

Polyline<Inexact> CubicBezierSpline::polyline(int nEdgesPerCurve) const {
	Polyline<Inexact> pl;
	if (nEdgesPerCurve < 1) return pl;

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
}
