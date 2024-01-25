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
#include "collapse.h"
#include <random>

namespace cartocrow::isoline_simplification {
IsolineSimplifier::IsolineSimplifier(std::vector<Isoline<K>> isolines): m_isolines(std::move(isolines)), m_simplified_isolines(m_isolines) {
	initialize_point_data();
	initialize_sdg();
	m_separator = medial_axis_separator(m_delaunay, m_p_isoline, m_p_prev, m_p_next);
	m_matching = matching(m_delaunay, m_separator, m_p_prev, m_p_next, m_p_isoline, true);
	m_slope_ladders = slope_ladders(m_matching, m_simplified_isolines, m_p_next);
}

void IsolineSimplifier::initialize_point_data() {
	for (auto& isoline : m_simplified_isolines) {
		for (auto pit = isoline.m_points.begin(); pit != isoline.m_points.end(); pit++) {
			Gt::Point_2& p = *pit;
			m_p_isoline[p] = &isoline;
			m_p_iterator[p] = pit;
			if (pit != isoline.m_points.begin()) {
				auto prev = pit;
				prev--;
				m_p_prev[p] = *prev;
			} else if (isoline.m_closed) {
				m_p_prev[p] = isoline.m_points.back();
			}
			if (pit != --isoline.m_points.end()) {
				auto next = pit;
				next++;
				m_p_next[p] = *next;
			} else if (isoline.m_closed) {
				m_p_next[p] = isoline.m_points.front();
			}
		}
	}
}

void IsolineSimplifier::initialize_sdg() {
	for (const auto& isoline : m_isolines) {
		auto polyline = isoline.polyline();
		m_delaunay.insert_segments(polyline.edges_begin(), polyline.edges_end());
	}
}

std::vector<Isoline<K>> IsolineSimplifier::simplify() {
	return m_isolines;
}

void IsolineSimplifier::collapse_edge(Gt::Segment_2 edge, Gt::Point_2 new_vertex) {
	const auto& a = edge.source();
	const auto& b = edge.target();
	auto a_it = m_p_iterator.at(a);
	auto b_it = m_p_iterator.at(b);
	Isoline<K>* a_iso = m_p_isoline.at(a);
	Isoline<K>* b_iso = m_p_isoline.at(b);
	assert(a_iso == b_iso);
	assert(m_p_next[a] == b && m_p_prev[b] == a || m_p_next[b] == a && m_p_prev[a] == b);

	bool reversed = m_p_next[b] == a;

	// Remove points from isolines
	auto new_it = a_iso->m_points.insert(b_it, new_vertex);
	a_iso->m_points.erase(a_it);
	b_iso->m_points.erase(b_it);

	// Update m_p_isoline
	m_p_isoline[new_vertex] = a_iso;
	m_p_isoline.erase(a);
	m_p_isoline.erase(b);

	// Update m_p_iterator
	m_p_iterator[new_vertex] = new_it;
	m_p_iterator.erase(a);
	m_p_iterator.erase(b);

	// Update Delaunay (may not be necessary) 1
	const auto& seg_vertex = m_delaunay.nearest_neighbor(midpoint(edge));
	m_delaunay.remove(seg_vertex);

	if (!reversed) {
		if (m_p_prev.contains(a)) {
			const auto& a_seg = m_delaunay.nearest_neighbor(CGAL::midpoint(m_p_prev.at(a), a));
			m_delaunay.remove(a_seg);
		}
		if (m_p_next.contains(b)) {
			const auto& b_seg = m_delaunay.nearest_neighbor(CGAL::midpoint(b, m_p_next.at(b)));
			m_delaunay.remove(b_seg);
		}
	} else {
		if (m_p_prev.contains(b)) {
			const auto& b_seg = m_delaunay.nearest_neighbor(CGAL::midpoint(m_p_prev.at(b), b));
			m_delaunay.remove(b_seg);
		}
		if (m_p_next.contains(a)) {
			const auto& a_seg = m_delaunay.nearest_neighbor(CGAL::midpoint(a, m_p_next.at(a)));
			m_delaunay.remove(a_seg);
		}
	}
	const auto& a_pnt = m_delaunay.nearest_neighbor(a);
	m_delaunay.remove(a_pnt);
	const auto& b_pnt = m_delaunay.nearest_neighbor(b);
	m_delaunay.remove(b_pnt);

	// Update prev and next
	if (!reversed) {
		if (m_p_prev.contains(a)) {
			const auto prev = m_p_prev[a];
			m_p_next[prev] = new_vertex;
			m_p_prev[new_vertex] = prev;
			m_p_prev.erase(a);
			m_p_next.erase(a);
		}
		if (m_p_next.contains(b)) {
			const auto next = m_p_next[b];
			m_p_prev[next] = new_vertex;
			m_p_next[new_vertex] = next;
			m_p_prev.erase(b);
			m_p_next.erase(b);
		}
	} else {
		if (m_p_prev.contains(b)) {
			const auto prev = m_p_prev[b];
			m_p_next[prev] = new_vertex;
			m_p_prev[new_vertex] = prev;
			m_p_prev.erase(b);
			m_p_next.erase(b);
		}
		if (m_p_next.contains(a)) {
			const auto next = m_p_next[a];
			m_p_prev[next] = new_vertex;
			m_p_next[new_vertex] = next;
			m_p_prev.erase(a);
			m_p_next.erase(a);
		}
	}

	// Update Delaunay (may not be necessary) 2
	if (m_p_prev.contains(new_vertex)) {
		m_delaunay.insert(SDG2::Site_2::construct_site_2(m_p_prev.at(new_vertex), new_vertex));
	}
	if (m_p_next.contains(new_vertex)) {
		m_delaunay.insert(SDG2::Site_2::construct_site_2(new_vertex, m_p_next.at(new_vertex)));
	}
	m_delaunay.insert(SDG2::Site_2::construct_site_2(new_vertex));
}

void IsolineSimplifier::update_separator() {
	// Naive slow: recompute
	m_separator = medial_axis_separator(m_delaunay, m_p_isoline, m_p_prev, m_p_next);
}

void IsolineSimplifier::update_matching() {
	// Naive slow: recompute
	m_matching = matching(m_delaunay, m_separator, m_p_prev, m_p_next, m_p_isoline, true);
}

void IsolineSimplifier::update_ladders() {
	// Naive slow: recompute
	m_slope_ladders = slope_ladders(m_matching, m_simplified_isolines, m_p_next);
}

void IsolineSimplifier::step() {
	static std::mt19937 gen(0);
	std::uniform_int_distribution dist(0, static_cast<int>(m_slope_ladders.size() - 1));
	int index = dist(gen);
	auto slope_ladder = m_slope_ladders[index];
	auto new_pts = collapse(slope_ladder, m_p_prev, m_p_next);
	for (int i = 0; i < slope_ladder.rungs.size(); i++) {
		collapse_edge(slope_ladder.rungs[i], new_pts[i]);
	}

	// TODO:
	// - remove vertices and segments of old slope ladder
	// - add new vertices and segments
	// - update all data
	// Probably refactor simplified isoline to std::list, then store iterators etc. no need for prev and next maps.
	// This also allows for more efficient removal.
}
}