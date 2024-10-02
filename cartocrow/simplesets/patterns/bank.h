#ifndef CARTOCROW_BANK_H
#define CARTOCROW_BANK_H

#include "pattern.h"
#include "poly_pattern.h"

namespace cartocrow::simplesets {
struct Bend {
	CGAL::Orientation orientation;
	Number<Inexact> maxAngle;
	Number<Inexact> totalAngle;
	int startIndex;
	int endIndex;
	Bend(CGAL::Orientation orientation, Number<Inexact> maxAngle, Number<Inexact> totalAngle, int startIndex, int endIndex) :
	  orientation(orientation), maxAngle(maxAngle), totalAngle(totalAngle), startIndex(startIndex), endIndex(endIndex){}
};

class Bank : public PolyPattern {
  public:
	Bank(std::vector<CatPoint> catPoints);

	std::variant<Polyline<Inexact>, Polygon<Inexact>> poly() const override;
	const std::vector<CatPoint>& catPoints() const override;
	Number<Inexact> coverRadius() const override;
	bool isValid(const GeneralSettings& gs) const;

  private:
	std::vector<CatPoint> m_catPoints;
	std::vector<Point<Inexact>> m_points;
	Number<Inexact> m_coverRadius;
	Polyline<Inexact> m_polyline;
	std::vector<Bend> m_bends;

	void computeBends();
};
}

#endif //CARTOCROW_BANK_H
