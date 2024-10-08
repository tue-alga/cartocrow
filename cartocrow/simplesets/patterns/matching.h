#ifndef CARTOCROW_MATCHING_H
#define CARTOCROW_MATCHING_H

#include "pattern.h"
#include "poly_pattern.h"

namespace cartocrow::simplesets {
class Matching : public PolyPattern {
  public:
	Matching(const CatPoint& catPoint1, const CatPoint& catPoint2);
	std::variant<Polyline<Inexact>, Polygon<Inexact>> poly() const override;
	const std::vector<CatPoint>& catPoints() const override;
	Number<Inexact> coverRadius() const override;

  private:
	std::vector<CatPoint> m_catPoints;
};
}
#endif //CARTOCROW_MATCHING_H
