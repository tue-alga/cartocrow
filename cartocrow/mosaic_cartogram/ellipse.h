#ifndef CARTOCROW_MOSAIC_CARTOGRAM_ELLIPSE_H
#define CARTOCROW_MOSAIC_CARTOGRAM_ELLIPSE_H

#include <string>
#include <utility>
#include <Eigen/Dense>

#include "../core/core.h"

namespace cartocrow::mosaic_cartogram {

// TODO: move to core?

class EllipseAtOrigin;

class Ellipse {
  protected:
	double A, B, C, D, E, F;

  public:
	struct Parameters {
		double a, b, angle, x0, y0;
		Eigen::Matrix3d matrix() const;
	};

	Ellipse() : A(1), B(), C(1), D(), E(), F(-1) {}
	Ellipse(double a, double b, double c, double d, double e, double f);

	double angle() const;
	Eigen::Vector2d center() const;
	std::array<double, 6> coefficients() const;
	double evaluate(double x, double y) const;
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

class EllipseAtOrigin : public Ellipse {
  public:
	EllipseAtOrigin() : Ellipse() {}
	EllipseAtOrigin(double a, double b, double c, double f) : Ellipse(a, b, c, 0, 0, f) {}

	double area() const;
	Parameters parameters() const override;
	double radius(double slope) const;
	EllipseAtOrigin scaleTo(double area) const;
};

} // namespace cartocrow::mosaic_cartogram

#endif // CARTOCROW_MOSAIC_CARTOGRAM_ELLIPSE_H
