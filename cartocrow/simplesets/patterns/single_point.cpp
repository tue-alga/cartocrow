#include "single_point.h"

namespace cartocrow::simplesets {
SinglePoint::SinglePoint(CatPoint catPoint): m_catPoints({catPoint}) {}

std::variant<Polyline<Inexact>, Polygon<Inexact>> SinglePoint::poly() const {
	std::vector<Point<Inexact>> pts({m_catPoints[0].point});
	return Polygon<Inexact>(pts.begin(), pts.end());
}

const std::vector<CatPoint>& SinglePoint::catPoints() const {
	return m_catPoints;
}

Number<Inexact> SinglePoint::coverRadius() const {
	return 0;
}

CatPoint SinglePoint::catPoint() const {
	return m_catPoints[0];
}
}