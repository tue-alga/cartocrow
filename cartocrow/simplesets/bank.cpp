#include "bank.h"

namespace cartocrow::simplesets {
Bank::Bank(std::vector<CatPoint> catPoints): m_catPoints(std::move(catPoints)) {
	// Store the point positions separately, sometimes only the positions are needed.
	std::transform(m_catPoints.begin(), m_catPoints.end(), std::back_inserter(m_points), [](const CatPoint& cp) {
		return cp.point;
	});

	// Compute the cover radius
	std::optional<Number<K>> maxSquaredDistance;
	for (int i = 0; i < m_points.size() - 1; i++) {
		auto p = m_points[i];
		auto q = m_points[i+1];
		auto dist = squared_distance(p, q);
		if (!maxSquaredDistance.has_value() || dist > *maxSquaredDistance) {
			maxSquaredDistance = dist;
		}
	}
	m_coverRadius = sqrt(*maxSquaredDistance) / 2;

	// Compute bends
	computeBends();
}

Number<K> approximateAngleBetween(const Vector<K>& exact_v, const Vector<K>& exact_w) {
	auto v = approximate(exact_v);
	auto w = approximate(exact_w);
	return acos((v * w) / (sqrt(v.squared_length()) * sqrt(w.squared_length())));
}

void Bank::computeBends() {
	m_bends.clear();

	std::optional<CGAL::Orientation> orientation;
	Number<K> bendTotalAngle = 0;
	Number<K> bendMaxAngle = 0;
	int startIndex = 0;

	for (int i = 0; i < m_points.size(); i++) {
		if (i + 2 > m_points.size() - 1) break;
		auto orient = CGAL::orientation(m_points[i], m_points[i+1], m_points[i+2]);
		auto angle = approximateAngleBetween(m_points[i+1] - m_points[i], m_points[i+2] - m_points[i+1]);
		if (orientation == -orient) {
			// Switched orientation
			m_bends.emplace_back(*orientation, bendMaxAngle, bendTotalAngle, startIndex, i+1);
			orientation = orient;
			bendTotalAngle = angle;
			bendMaxAngle = angle;
			startIndex = i;
		} else {
			orientation = orient;
			bendTotalAngle += angle;
			bendMaxAngle = std::max(bendMaxAngle, angle);
		}
	}

	if (orientation.has_value()) {
		m_bends.emplace_back(*orientation, bendMaxAngle, bendTotalAngle, startIndex, static_cast<int>(m_points.size()-1));
	}
}

std::variant<Polyline<K>, Polygon<K>> Bank::contour() {
	return m_polyline;
}

std::vector<CatPoint> Bank::catPoints() {
	return m_catPoints;
}

bool Bank::isValid(GeneralSettings gs) {
	bool inflectionIsFine = m_bends.size() <= gs.inflectionLimit;
	bool anglesAreFine = true;
	for (const auto& bend : m_bends) {
		anglesAreFine = anglesAreFine && bend.maxAngle <= gs.maxTurnAngle;
	}
	bool totalAngleIsFine = true;
	for (const auto& bend : m_bends) {
		if (bend.totalAngle > gs.maxBendAngle) {
			totalAngleIsFine = false;
		}
	}
	return inflectionIsFine && anglesAreFine && totalAngleIsFine;
}

Number<K> Bank::coverRadius() {
	return m_coverRadius;
}
}