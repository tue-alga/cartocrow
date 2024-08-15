#include "matching.h"

namespace cartocrow::simplesets {
Matching::Matching(const CatPoint& catPoint1, const CatPoint& catPoint2) : m_catPoints({catPoint1, catPoint2}) {}

std::variant<Polyline<Inexact>, Polygon<Inexact>> Matching::poly() const {
//	std::vector<Point<Inexact>> pts({m_catPoints.first.point, m_catPoints.second.point});
//	return Polygon<Inexact>(pts.begin(), pts.end());
	return Polyline<Inexact>(std::vector({m_catPoints[0].point, m_catPoints[1].point}));
}

const std::vector<CatPoint>& Matching::catPoints() const {
	return m_catPoints;
}

Number<Inexact> Matching::coverRadius() const {
	return sqrt(squared_distance(m_catPoints[0].point, m_catPoints[1].point)) / 2;
}
}
