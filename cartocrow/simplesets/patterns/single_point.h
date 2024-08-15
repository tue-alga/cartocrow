#ifndef CARTOCROW_SINGLE_POINT_H
#define CARTOCROW_SINGLE_POINT_H

#include "pattern.h"
#include "poly_pattern.h"

namespace cartocrow::simplesets {
class SinglePoint : public PolyPattern {
  public:
	SinglePoint(CatPoint catPoint);
	std::variant<Polyline<Inexact>, Polygon<Inexact>> poly() const override;
	const std::vector<CatPoint>& catPoints() const override;
	Number<Inexact> coverRadius() const override;
	CatPoint catPoint() const;

  private:
	std::vector<CatPoint> m_catPoints;
};
}

#endif //CARTOCROW_SINGLE_POINT_H
