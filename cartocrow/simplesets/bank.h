#ifndef CARTOCROW_BANK_H
#define CARTOCROW_BANK_H

#include "pattern.h"

namespace cartocrow::simplesets {
struct Bend {
	CGAL::Orientation orientation;
	Number<K> maxAngle;
	Number<K> totalAngle;
	int startIndex;
	int endIndex;
};

class Bank : public Pattern {
  public:
	Bank(std::vector<CatPoint> catPoints);

	std::variant<Polyline<K>, Polygon<K>> contour() override;
	std::vector<CatPoint> catPoints() override;
	Number<K> coverRadius() override;
	bool isValid(GeneralSettings gs) override;

  private:
	std::vector<CatPoint> m_catPoints;
	std::vector<Point<K>> m_points;
	Number<K> m_coverRadius;
	Polygon<K> m_polyline;
	std::vector<Bend> m_bends;

	void computeBends();
};
}

#endif //CARTOCROW_BANK_H
