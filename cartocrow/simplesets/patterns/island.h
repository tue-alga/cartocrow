#ifndef CARTOCROW_ISLAND_H
#define CARTOCROW_ISLAND_H

#include "pattern.h"
#include "poly_pattern.h"
#include <CGAL/Delaunay_triangulation_2.h>

namespace cartocrow::simplesets {
typedef CGAL::Delaunay_triangulation_2<Exact> DT;

class Island : public PolyPattern {
  public:
	Island(std::vector<CatPoint> catPoints);

	std::variant<Polyline<Inexact>, Polygon<Inexact>> poly() const override;
	const std::vector<CatPoint>& catPoints() const override;
	Number<Inexact> coverRadius() const override;

  private:
	std::vector<CatPoint> m_catPoints;
	std::vector<Point<Inexact>> m_points;
	Number<Inexact> m_coverRadius;
	std::variant<Polyline<Inexact>, Polygon<Inexact>> m_poly;
};
}

#endif //CARTOCROW_ISLAND_H
