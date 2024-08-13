#ifndef CARTOCROW_ISLAND_H
#define CARTOCROW_ISLAND_H

#include "pattern.h"
#include <CGAL/Delaunay_triangulation_2.h>

namespace cartocrow::simplesets {
typedef CGAL::Delaunay_triangulation_2<K> DT;

class Island : public Pattern {
  public:
	Island(std::vector<CatPoint> catPoints);

	std::variant<Polyline<K>, Polygon<K>> contour() override;
	std::vector<CatPoint> catPoints() override;
	Number<K> coverRadius() override;
	bool isValid(GeneralSettings gs) override;

  private:
	std::vector<CatPoint> m_catPoints;
	std::vector<Point<K>> m_points;
	Number<K> m_coverRadius;
	Polygon<K> m_polygon;
};
}

#endif //CARTOCROW_ISLAND_H
