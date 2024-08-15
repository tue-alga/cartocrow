#include "ellipse.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <numbers>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <CGAL/number_utils.h>
#include <Eigen/Eigenvalues>

#include "../core/centroid.h"

namespace cartocrow {

bool isZero(double x) {
	return std::abs(x) < 1e-10;
}

Eigen::Matrix3d Ellipse::Parameters::matrix() const {
	// multiply matrices left to right: translation, rotation, scaling
	const double cos = std::cos(angle);
	const double sin = std::sin(angle);
	return Eigen::Matrix3d {
		{ cos*a, -sin*b, x0 },
		{ sin*a,  cos*b, y0 },
		{ 0,      0,     1  }
	};
}

// TODO: only enable checks in debug builds?
Ellipse::Ellipse(double a, double b, double c, double d, double e, double f)
    : A(a), B(b), C(c), D(d), E(e), F(f) {
	if (!std::isfinite(a) || !std::isfinite(b) || !std::isfinite(c) ||
	    !std::isfinite(d) || !std::isfinite(e) || !std::isfinite(f)) {
		throw std::invalid_argument("The coefficients cannot be infinite or NaN");
	}

	// see Lawrence (1972), page 63
	const double min = 4*A*C - B*B;
	const double det = min * F - A*E*E + (B*E - C*D) * D;
	if (det == 0) {
		throw std::invalid_argument("The conic cannot be degenerate");
	}
	if (min <= 0) {
		throw std::invalid_argument("The conic cannot be a hyperbola or parabola");
	}
	if (A*det >= 0) {
		throw std::invalid_argument("The ellipse cannot be imaginary");
	}
}

double Ellipse::angle() const {
	// substitute  x -> x cos(t) - y sin(t)  and  y -> x sin(t) + y cos(t)
	// and solve for t such that coefficient of xy is 0
	if (isZero(A - C)) return std::numbers::pi / 2;
	return .5 * std::atan(B / (A-C));
}

Point<Inexact> Ellipse::center() const {
	// solve system  { dQ/dx = 0, dQ/dy = 0 }  for  x,y
	const double x = (2*C*D - B*E) / (B*B - 4*A*C);
	return { x, -(B*x + E) / (2*C) };
}

Ellipse Ellipse::normalizeSign() const {
	if (A > 0) return *this;  // note:  sgn(A) = sgn(C)
	return { -A, -B, -C, -D, -E, -F };
}

Ellipse::Parameters Ellipse::parameters() const {
	Parameters p = translateToOrigin().parameters();
	const Point<Inexact> c = center();
	p.x0 = c.x(), p.y0 = c.y();
	return p;
}

Ellipse Ellipse::stretch(double cx, double cy) const {
	// substitute  x -> cx x  and  y -> cy y  and compute new coefficients
	return { A*cx*cx, B*cx*cy, C*cy*cy, D*cx, E*cy, F };
}

Ellipse Ellipse::translate(double dx, double dy) const {
	// substitute  x -> x-dx  and  y -> y-dy  and compute new coefficients
	return {
		A, B, C,
		D - 2*A*dx - B*dy,
		E - 2*C*dy - B*dx,
		(A*dx - D) * dx + (B*dx + C*dy - E) * dy + F
	};
}

Ellipse Ellipse::translateTo(double x, double y) const {
	const Point<Inexact> c = center();
	return translate(x - c.x(), y - c.y());
}

EllipseAtOrigin Ellipse::translateToOrigin() const {
	const Point<Inexact> c = center();
	const double x0 = c.x(), y0 = c.y();
	return {
		A, B, C,
		(A*x0 + D) * x0 + (B*x0 + C*y0 + E) * y0 + F
	};
}

std::ostream& operator<<(std::ostream &os, const Ellipse &e) {
	const std::array<double, 6> coefficients = e.coefficients();
	static const std::vector<std::string> variables = { "x²", "xy", "y²", "x", "y", "" };

	bool first = true;
	for (int i = 0; i < coefficients.size(); i++) {
		double c = coefficients[i];
		const std::string &v = variables[i];

		if (!c) continue;
		if (!first) {
			os << (c > 0 ? " + " : " - ");
			c = std::abs(c);
		}
		if (c != 1 || v.empty()) os << c;
		os << v;

		first = false;
	}
	return os << " = 0";
}

std::string Ellipse::toString(int precision) const {
	std::stringstream ss;
	ss << std::setprecision(precision) << *this;
	return ss.str();
}

template <class K> Ellipse Ellipse::fit(const Polygon<K> &polygon) {
	const auto n = polygon.size();

	// if the polygon is too small, we simply return a circle with the same area and centroid
	if (n < 6) {
		const EllipseAtOrigin circle(1, 0, 1, -CGAL::to_double(polygon.area()) / std::numbers::pi);
		return circle.translate(approximate(centroid(polygon)) - CGAL::ORIGIN);
	}

	// TODO: linearly space points on boundary instead of taking vertices, which biases the ellipse towards "detailed corners"
	Eigen::ArrayX2d boundary(n, 2);
	for (int i = 0; i < n; i++) {
		const Point<K> &p = polygon.vertex(i);
		boundary(i, 0) = CGAL::to_double(p.x());
		boundary(i, 1) = CGAL::to_double(p.y());
	}
	return fit(boundary);
}

// instantiate template function for Exact and Inexact
template Ellipse Ellipse::fit<Exact>(const Polygon<Exact> &polygon);
template Ellipse Ellipse::fit<Inexact>(const Polygon<Inexact> &polygon);

// based on Fitzgibbon et al. (1999)
// TODO: implement improvements by Harker et al. (2008)?
Ellipse leastSquares(const Eigen::ArrayX2d &boundary) {
	const int n = boundary.rows();
	if (n < 6) throw std::invalid_argument("To fit an ellipse you need at least 6 points");

	const Eigen::ArrayXd xs = boundary.col(0);
	const Eigen::ArrayXd ys = boundary.col(1);

	// design matrix (n×6)
	Eigen::MatrixXd D(n, 6);
	D << xs*xs, xs*ys, ys*ys, xs, ys, Eigen::VectorXd::Ones(n);

	// scatter matrix (6×6)
	const Eigen::MatrixXd S = D.transpose() * D;

	// constraint matrix (6×6)
	Eigen::MatrixXd C = Eigen::MatrixXd::Zero(6, 6);
	C(0, 2) =  2;
	C(1, 1) = -1;
	C(2, 0) =  2;

	// Compute the one positive eigenvalue and corresponding eigenvector. Due to numerical instabi-
	// lity, there are often other positive eigenvalues that are very close to 0. We ignore these by
	// taking the largest.
	// TODO: What should happen with zero, infinite, or imaginary eigenvalues?
	// TODO: This could produce nonsense (e.g., see Halíř & Flusser, 1998).
	const Eigen::EigenSolver<Eigen::MatrixXd> evSolver(S.inverse() * C);
	const Eigen::Vector<double, 6> evs = evSolver.eigenvalues().real();
	const int i = std::max_element(evs.begin(), evs.end()) - evs.begin();
	const Eigen::Vector<double, 6> u = evSolver.eigenvectors().col(i).real();

	// ellipse coefficients
	const Eigen::Vector<double, 6> a = u / std::sqrt(u.transpose() * C * u);
	return { a[0], a[1], a[2], a[3], a[4], a[5] };
}

Ellipse Ellipse::fit(const Eigen::ArrayX2d &boundary) {
	Eigen::ArrayX2d ps = boundary;

	// normalize the points: subtract mean and divide by standard deviation
	// this improves numerical stability during fitting
	const Eigen::Array<double, 1, 2> mean = ps.colwise().mean();
	ps.rowwise() -= mean;
	const Eigen::Array<double, 1, 2> invStd = std::sqrt(ps.rows()) / ps.colwise().norm();
	ps.rowwise() *= invStd;

	// fit ellipse and "denormalize"
	return leastSquares(ps)
		.stretch(invStd[0], invStd[1])
		.translate(mean[0], mean[1]);
}

double EllipseAtOrigin::area() const {
	// https://math.stackexchange.com/q/2499695
	return 2 * std::numbers::pi * std::abs(F) / std::sqrt(4*A*C - B*B);
}

EllipseAtOrigin EllipseAtOrigin::normalizeContours(double deltaArea) const {
	// let  g(x) = area(Q + x)  and solve  g'(1) = deltaArea  for (scaling factor of) new coefficients
	const double c = 2 * std::numbers::pi / (std::sqrt(4*A*C - B*B) * deltaArea);
	return { c*A, c*B, c*C, c*F };
}

Ellipse::Parameters EllipseAtOrigin::parameters() const {
	Parameters p;
	p.angle = angle();
	p.x0 = p.y0 = 0;

	const double cos = std::cos(p.angle);
	const double sin = std::sin(p.angle);

	// substitute  x -> x cos(t) - y sin(t)  and  y -> x sin(t) + y cos(t)  where  t := angle
	// this eliminates rotation and transforms ellipse into the form  Ax² + Cy² + F = 0
	const double A2 = A*cos*cos + ( B*cos + C*sin) * sin;
	const double C2 = C*cos*cos + (-B*cos + A*sin) * sin;

	p.a = std::sqrt(-F / A2);
	p.b = std::sqrt(-F / C2);

	return p;
}

double EllipseAtOrigin::radius(double slope) const {
	// special case: vertical line intersection
	if (std::isinf(slope)) return std::sqrt(-F / C);

	// substitute  y -> mx  i.e. the linear function with slope m through the origin
	// and rewrite as quadratic equation in x, i.e., ax² + 0x + F, where:
	const double a = A + (B + C*slope) * slope;
	// then  x = \pm\sqrt{-F/a}  so:
	const double xSq = -F / a;
	// return distance from origin to  (x, y) = (x, mx)
	return std::sqrt(xSq * (1 + slope*slope));
}

EllipseAtOrigin EllipseAtOrigin::scaleTo(double area) const {
	// note that area is linear in |F|, so  areaNew / areaOld = |Fnew| / |Fold| = Fnew / Fold
	// since the sign must remain the same
	return { A, B, C, F * area / this->area() };
}

} // namespace cartocrow
