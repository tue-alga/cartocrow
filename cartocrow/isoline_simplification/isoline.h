/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
Copyright (C) 2024 TU Eindhoven

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef CARTOCROW_ISOLINE_H
#define CARTOCROW_ISOLINE_H

#include "../core/core.h"

namespace cartocrow::isoline_simplification {
/// An object of this class represent an isoline.
/// This can be viewed as wrapper around the \ref Polyline and \ref Polygon classes.
/// The points are stored as a public linked list for convenient and efficient removal and insertion.
template <class K>
class Isoline {
  public:
	Isoline() = default;

	Isoline(std::vector<Point<K>> points, bool closed)
		: m_points(points.begin(), points.end()), m_closed(closed) {

	}

	std::list<Point<K>> m_points;
	bool m_closed;

	[[nodiscard]] Polygon<K> polygon() const { return std::get<Polygon<K>>(drawing_representation()); }

	Polyline<K> polyline() const {
		Polyline<K> pl(m_points.begin(), m_points.end());
		if (m_closed)
			pl.push_back(m_points.front());
		return pl;
	}

	std::variant<Polyline<K>, Polygon<K>> drawing_representation() const {
		if (m_closed) {
			return Polygon<K>(m_points.begin(), m_points.end());
		} else {
			return polyline();
		}
	}
};
}

#endif //CARTOCROW_ISOLINE_H
