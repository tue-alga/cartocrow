/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
Copyright (C) 2021  Netherlands eScience Center and TU Eindhoven

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3f of the License, or
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
    template <class K>
    class Isoline {
	  public:
	    Isoline(std::vector<Point<K>> points, bool closed)
	        : m_points(std::move(points)), m_closed(closed), m_polyline(m_points) {
		    if (closed) {
			    m_polyline.push_back(m_points.front());
			    m_drawing_representation = Polygon<K>(m_points.begin(), m_points.end());
		    } else {
			    m_drawing_representation = m_polyline;
		    }
	    }

	    std::vector<Point<K>> m_points;
	    bool m_closed;

	    std::variant<Polyline<K>, Polygon<K>> m_drawing_representation;

	    Polyline<K> m_polyline;

	    [[nodiscard]] Polyline<K>::Edge_iterator edges_begin() const {
		    return m_polyline.edges_begin();
	    }

	    [[nodiscard]] Polyline<K>::Edge_iterator edges_end() const {
		    return m_polyline.edges_end();
	    }

	    [[nodiscard]] Polygon<K> polygon() const { return std::get<Polygon<K>>(m_drawing_representation); }
    };
}

#endif //CARTOCROW_ISOLINE_H
