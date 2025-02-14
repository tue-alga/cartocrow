#ifndef CARTOCROW_DILATED_POLY_H
#define CARTOCROW_DILATED_POLY_H

#include "../patterns/poly_pattern.h"

namespace cartocrow::simplesets {
class DilatedPoly : public Pattern {
  public:
	DilatedPoly(const PolyPattern& polyPattern, const Number<Exact>& dilationRadius);
	std::variant<Polyline<Inexact>, Polygon<Inexact>, CSPolygon> contour() const override;
	const std::vector<CatPoint>& catPoints() const override;
	CSPolygon m_contour;

  private:
	std::vector<CatPoint> m_catPoints;
};
}
#endif //CARTOCROW_DILATED_POLY_H
