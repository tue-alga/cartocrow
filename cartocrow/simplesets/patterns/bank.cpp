#include "bank.h"

namespace cartocrow::simplesets {
Bank::Bank(std::vector<CatPoint> catPoints): m_catPoints(std::move(catPoints)) {
	// Store the point positions separately, sometimes only the positions are needed.
	std::transform(m_catPoints.begin(), m_catPoints.end(), std::back_inserter(m_points), [](const CatPoint& cp) {
		return cp.point;
	});

	// Store polyline
	m_polyline = Polyline<Inexact>(m_points);

	// Compute the cover radius
	std::optional<Number<Inexact>> maxSquaredDistance;
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

Number<Inexact> computeAngleBetween(const Vector<Inexact>& v, const Vector<Inexact>& w) {
	return acos((v * w) / (sqrt(v.squared_length()) * sqrt(w.squared_length())));
}

void Bank::computeBends() {
	m_bends.clear();

	std::optional<CGAL::Orientation> orientation;
	Number<Inexact> bendTotalAngle = 0;
	Number<Inexact> bendMaxAngle = 0;
	int startIndex = 0;

	for (int i = 0; i < m_points.size(); i++) {
		if (i + 2 > m_points.size() - 1) break;
		auto orient = CGAL::orientation(m_points[i], m_points[i+1], m_points[i+2]);
		auto angle = computeAngleBetween(m_points[i+1] - m_points[i], m_points[i+2] - m_points[i+1]);
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

std::variant<Polyline<Inexact>, Polygon<Inexact>> Bank::poly() const {
	return m_polyline;
}

const std::vector<CatPoint>& Bank::catPoints() const {
	return m_catPoints;
}

bool Bank::isValid(const GeneralSettings& gs) const {
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

Number<Inexact> Bank::coverRadius() const {
	return m_coverRadius;
}
}