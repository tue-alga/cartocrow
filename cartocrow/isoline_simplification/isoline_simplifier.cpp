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

#include "isoline_simplifier.h"
#include "medial_axis_separator.h"

namespace cartocrow::isoline_simplification {
IsolineSimplifier::IsolineSimplifier(std::vector<Isoline<K>> isolines): m_isolines(std::move(isolines)) {
	initialize_point_data();
	initialize_sdg();
	m_separator = medial_axis_separator(m_delaunay, m_p_isoline, m_p_prev, m_p_next);
	m_matching = matching(m_delaunay, m_separator, m_p_prev, m_p_next, m_p_isoline, m_p_index);
	m_slope_ladders = slope_ladders(m_matching, m_isolines, m_p_next);
}

void IsolineSimplifier::initialize_point_data() {
	for (auto& isoline : m_isolines) {
		for (int i = 0; i < isoline.m_points.size(); i++) {
			Gt::Point_2& p = isoline.m_points[i];
			m_p_isoline[p] = &isoline;
			m_p_index[p] = i;
			if (i > 0) {
				m_p_prev[p] = &isoline.m_points[i-1];
			} else if (isoline.m_closed) {
				m_p_prev[p] = &isoline.m_points.back();
			}
			if (i < isoline.m_points.size() - 1) {
				m_p_next[p] = &isoline.m_points[i+1];
			} else if (isoline.m_closed) {
				m_p_next[p] = &isoline.m_points.front();
			}
		}
	}
}

void IsolineSimplifier::initialize_sdg() {
	for (const auto& isoline : m_isolines) {
		m_delaunay.insert_segments(isoline.edges_begin(), isoline.edges_end());
	}
}

std::vector<Isoline<K>> IsolineSimplifier::simplify() {
	return m_isolines;
}
}