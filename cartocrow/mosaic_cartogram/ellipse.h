#ifndef CARTOCROW_MOSAIC_CARTOGRAM_ELLIPSE_H
#define CARTOCROW_MOSAIC_CARTOGRAM_ELLIPSE_H

#include <string>
#include <utility>
#include <Eigen/Dense>

#include "../core/core.h"

namespace cartocrow::mosaic_cartogram {

struct EllipseAtOrigin;

struct Ellipse {
	struct Parameters {
		double a, b, angle, x0, y0;
		Eigen::Matrix3d matrix() const;
	};

	const double A, B, C, D, E, F;

	Ellipse(double a, double b, double c, double d, double e, double f);

	double angle() const;
	std::pair<double, double> center() const;
	virtual Parameters parameters() const;
	Ellipse stretch(double cx, double cy) const;
	Ellipse translate(double dx, double dy) const;
	Ellipse translateTo(double x, double y) const;
	EllipseAtOrigin translateToOrigin() const;

	friend std::ostream& operator<<(std::ostream &os, const Ellipse &e);
	std::string toString(int precision = 3) const;

	template <class K>
	static Ellipse fit(const Polygon<K> &polygon);
	static Ellipse fit(const Eigen::ArrayX2d &boundary);
};

struct EllipseAtOrigin : public Ellipse {
	using Ellipse::A;
	using Ellipse::B;
	using Ellipse::C;
	using Ellipse::F;  // guaranteed to be strictly negative

	EllipseAtOrigin(double a, double b, double c, double f) : Ellipse(a, b, c, 0, 0, f) {}

	double area() const;
	Parameters parameters() const override;
	EllipseAtOrigin scaleTo(double area) const;
};

} // namespace cartocrow::mosaic_cartogram

#endif // CARTOCROW_MOSAIC_CARTOGRAM_ELLIPSE_H
