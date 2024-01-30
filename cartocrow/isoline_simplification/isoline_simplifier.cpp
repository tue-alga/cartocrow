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
auto slope_ladder_comp = [](const std::shared_ptr<SlopeLadder>& sl1, const std::shared_ptr<SlopeLadder>& sl2) {
	return sl1->m_cost > sl2->m_cost;
};

IsolineSimplifier::IsolineSimplifier(std::vector<Isoline<K>> isolines): m_isolines(std::move(isolines)), m_simplified_isolines(m_isolines) {
	initialize_sdg();
	initialize_point_data();
	m_separator = medial_axis_separator(m_delaunay, m_p_isoline, m_p_prev, m_p_next);
	m_matching = matching(m_delaunay, m_separator, m_p_prev, m_p_next, m_p_isoline);
	initialize_slope_ladders();
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

	for (auto vit = m_delaunay.finite_vertices_begin(); vit != m_delaunay.finite_vertices_end(); vit++) {
		auto site = vit->site();
		if (site.is_point()) {
			m_p_vertex[site.point()] = vit;
		} else {
			m_e_vertex[site.segment()] = vit;
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

void IsolineSimplifier::collapse_edge(Gt::Segment_2 edge, Gt::Point_2 new_point) {
	auto insert_adj = [this](SDG2::Vertex_handle vertex) {
		// the vertices that are inserted can probably be reduced / limited by using the fact that
		// adjacent vertices of the same isoline do not affect the slope ladders.
		auto ic_start = m_delaunay.incident_vertices(vertex);
		auto ic = ic_start;
		if (ic != nullptr) {
			do {
				if (ic->storage_site().is_defined()) {
					m_changed_vertices.insert(ic);
				} else {
//					std::cout << "Encountered undefined storage site" << std::endl;
				}
			} while (++ic != ic_start);
		}
	};

	auto delaunay_remove_p = [&insert_adj, this](const Gt::Point_2& p) {
	 	auto vertex = m_p_vertex.at(p);
	 	insert_adj(vertex);
	  	m_changed_vertices.erase(vertex);
	 	if (!m_delaunay.remove(vertex)) {
			throw std::runtime_error("Delaunay removal failed!");
		}
	};

	auto delaunay_remove_e = [&insert_adj, this](const Gt::Segment_2& s) {
		auto seg = s;
		if (!m_e_vertex.contains(s)) {
			if (m_e_vertex.contains(s.opposite())) {
				seg = s.opposite();
			} else {
				throw std::runtime_error("Segment that should be removed is not part of the Delaunay graph!");
			}
		}
	    auto seg_vertex = m_e_vertex.at(seg);

	    insert_adj(seg_vertex);

	  	m_changed_vertices.erase(seg_vertex);
	    if (!m_delaunay.remove(seg_vertex)) {
			throw std::runtime_error("Delaunay removal failed!");
		}
	};

	auto delaunay_insert_p = [this, &insert_adj](const Gt::Point_2& p, const SDG2::Vertex_handle near) {
	  	auto handle = m_delaunay.insert(p, near);
	    m_changed_vertices.insert(handle);
	  	m_p_vertex[p] = handle;
	  	insert_adj(handle);
		return handle;
	};

	auto delaunay_insert_e = [this, &insert_adj](const Gt::Point_2& p1, const Gt::Point_2& p2, const SDG2::Vertex_handle near) {
	  	auto handle = m_delaunay.insert(p1, p2, near);
	  	m_e_vertex[Gt::Segment_2(p1, p2)] = handle;
		m_changed_vertices.insert(handle);
	  	insert_adj(handle);
		return handle;
	};

	bool reversed = m_p_next[edge.target()] == edge.source();
	const Gt::Point_2& t = reversed ? edge.target() : edge.source();
	const Gt::Point_2& u = reversed ? edge.source() : edge.target();
	assert(m_p_next[t] == u && m_p_prev[u] == t);
	const Gt::Point_2& s = m_p_prev[t];
	const Gt::Point_2& v = m_p_next[u];
	const Gt::Segment_2 st = Gt::Segment_2(s, t);
	const Gt::Segment_2 uv = Gt::Segment_2(u, v);

	m_deleted_points.push_back(t);
	m_deleted_points.push_back(u);

	auto t_it = m_p_iterator.at(t);
	auto u_it = m_p_iterator.at(u);
	Isoline<K>* t_iso = m_p_isoline.at(t);
	Isoline<K>* u_iso = m_p_isoline.at(u);
	assert(t_iso == u_iso);

	// Remove points from isolines
	auto new_it = t_iso->m_points.insert(u_it, new_point);
	t_iso->m_points.erase(t_it);
	u_iso->m_points.erase(u_it);

	// Update some ladder info
	if (m_e_ladder.contains(st)) {
		m_e_ladder.at(st)->m_old = true;
		m_e_ladder.erase(st);
	}
	if (m_e_ladder.contains(uv)) {
		m_e_ladder.at(uv)->m_old = true;
		m_e_ladder.erase(uv);
	}
	if (m_e_ladder.contains(st.opposite())) {
		m_e_ladder.at(st)->m_old = true;
		m_e_ladder.erase(st);
	}
	if (m_e_ladder.contains(uv.opposite())) {
		m_e_ladder.at(uv)->m_old = true;
		m_e_ladder.erase(uv);
	}

	// Update m_p_iterator
	m_p_iterator[new_point] = new_it;
	m_p_iterator.erase(t);
	m_p_iterator.erase(u);

	// Update Delaunay
	delaunay_remove_e(edge);
	delaunay_remove_e(st);
	delaunay_remove_e(uv);
	delaunay_remove_p(t);
	delaunay_remove_p(u);

	delaunay_insert_e(s, new_point, m_p_vertex.at(s));
	auto seg_handle = delaunay_insert_e(new_point, v, m_p_vertex.at(v));
	delaunay_insert_p(new_point, seg_handle);

	// Update m_p_isoline
	m_p_isoline[new_point] = t_iso;
	m_p_isoline.erase(t);
	m_p_isoline.erase(u);

	// Update prev and next
	m_p_next[s] = new_point;
	m_p_prev[new_point] = s;
	m_p_prev.erase(t);
	m_p_next.erase(t);
	m_p_prev[v] = new_point;
	m_p_next[new_point] = v;
	m_p_prev.erase(u);
	m_p_next.erase(u);
}

void IsolineSimplifier::update_separator() {
	// No need to update separator, todo: remove this method
//	m_separator = medial_axis_separator(m_delaunay, m_p_isoline, m_p_prev, m_p_next);
}

void IsolineSimplifier::update_matching() {
	std::unordered_set<Gt::Point_2> updated_points;
	std::unordered_set<SDG2::Vertex_handle> changed_vertices_and_endpoints;

	for (const auto& vh : m_changed_vertices) {
		const auto& site = vh->site();
		changed_vertices_and_endpoints.insert(vh);
		if (site.is_point()) {
			updated_points.insert(site.point());
		} else {
			const auto& seg = site.segment();
			updated_points.insert(seg.source());
			updated_points.insert(seg.target());
			changed_vertices_and_endpoints.insert(m_p_vertex.at(seg.source()));
			changed_vertices_and_endpoints.insert(m_p_vertex.at(seg.target()));
		}
	}

	for (const auto& p : m_deleted_points) {
		m_matching.erase(p);
	}

	for (const auto& p : updated_points) {
		if (!m_matching.contains(p)) continue;
		for (auto& [_, sign_map] : m_matching.at(p))
		for (auto& [_, pts] : sign_map)
		std::erase_if(pts, [&](const auto& item) {
			return updated_points.contains(item) || std::find(m_deleted_points.begin(), m_deleted_points.end(), item) != m_deleted_points.end();
		});
	}

	std::vector<Gt::Point_2> modified_matchings;
	for (const auto& vh : changed_vertices_and_endpoints) {
		auto site_1 = vh->site();
		auto iso_1 = m_p_isoline.at(point_of_site(site_1));

		auto ic_start = m_delaunay.incident_edges(vh);
		auto ic = ic_start;
		if (ic != nullptr) {
			do {
				auto edge = *ic;
				SDG2::Vertex_handle a = edge.first->vertex(SDG2::ccw(edge.second));
				SDG2::Vertex_handle b = edge.first->vertex(SDG2::cw(edge.second));
				SDG2::Vertex_handle target = a == vh ? b : a;

				if (!target->storage_site().is_defined()) continue;
				//	if (changed_vertices_and_endpoints.contains(target)) {
				auto site_2 = target->site();
				auto iso_2 = m_p_isoline.at(point_of_site(site_2));
				if (iso_1 == iso_2) continue;
				create_matching(m_delaunay, edge, m_matching, m_p_prev, m_p_next, m_p_isoline);
				if (site_1.is_point()) {
					modified_matchings.push_back(site_1.point());
				} else {
					auto seg = site_1.segment();
					modified_matchings.push_back(seg.source());
					modified_matchings.push_back(seg.target());
				}
				if (site_2.is_point()) {
					modified_matchings.push_back(site_2.point());
				} else {
					auto seg = site_2.segment();
					modified_matchings.push_back(seg.source());
					modified_matchings.push_back(seg.target());
				}
			} while (++ic != ic_start);
		}
	}

	for (const auto& pt : modified_matchings) {
		std::vector<CGAL::Sign> to_remove_s;
		for (auto& [sign, mi] : m_matching.at(pt)) {
			std::vector<Isoline<K>*> to_remove_i;
			for (auto& [iso, pts] : mi) {
				std::sort(pts.begin(), pts.end());//, comparison_f);
				pts.erase(std::unique(pts.begin(), pts.end()), pts.end());

				if (pts.empty()) {
					to_remove_i.push_back(iso);
				}
			}
			for (const auto& iso : to_remove_i)
				m_matching[pt][sign].erase(iso);

			if (mi.empty()) {
				to_remove_s.push_back(sign);
			}
		}
		for (const auto& sign : to_remove_s)
			m_matching[pt].erase(sign);
	}
}

void IsolineSimplifier::update_ladders() {
	// todo: fix bug in this ladder update code (recomputing from matching seems to work)
	if (!m_slope_ladders.empty()) {
		std::pop_heap(m_slope_ladders.begin(), m_slope_ladders.end(), slope_ladder_comp);
		m_slope_ladders.pop_back();
	}

	std::vector<Gt::Segment_2> additional_segments;

	for (const auto& vh : m_changed_vertices) {
		const auto& site = vh->site();
		if (site.is_segment()) {

			const auto& seg = site.segment();
			if (!m_e_ladder.contains(seg)) continue;
			m_e_ladder.at(seg)->m_old = true;
			m_e_ladder.erase(seg);
		}
	}

	for (const auto& vh : m_changed_vertices) {
		const auto& site = vh->site();
		if (site.is_point()) {
			const auto& p = site.point();
			if (!m_p_ladder.contains(p)) continue;
			for (auto& ladder : m_p_ladder.at(p)) {
				if (!ladder->m_old && (
				      ladder->m_cap.contains(CGAL::LEFT_TURN) && ladder->m_cap.at(CGAL::LEFT_TURN) == p ||
				      ladder->m_cap.contains(CGAL::RIGHT_TURN) && ladder->m_cap.at(CGAL::RIGHT_TURN) == p)) {
					additional_segments.push_back(ladder->m_rungs.front());
				}
				ladder->m_old = true;
			}
			m_p_ladder.erase(p);
		} else {
		}
	}

	for (const auto& vh : m_changed_vertices) {
		const auto& site = vh->site();
		if (site.is_point()) {
			auto p = site.point();
			if (m_p_prev.contains(p)) {
				create_slope_ladder(Gt::Segment_2(m_p_prev.at(p), p), true);
			}
			if (m_p_next.contains(p)) {
				create_slope_ladder(Gt::Segment_2(p, m_p_next.at(p)), true);
			}
		} else {
			create_slope_ladder(site.segment(), true);
		}
	}

	for (const auto& seg : additional_segments) {
		create_slope_ladder(seg, true);
	}
}

std::optional<std::shared_ptr<SlopeLadder>> IsolineSimplifier::next_ladder() {
	if (m_slope_ladders.empty()) return std::nullopt;

	// Note that m_slope_ladders is a heap
	auto& slope_ladder = m_slope_ladders.front();

	// Non-valid slope ladders have very high cost so this means no valid slope ladders are left
	if (!slope_ladder->m_valid) return std::nullopt;

	while (slope_ladder->m_old) {
		if (m_slope_ladders.empty()) return std::nullopt;
		std::pop_heap(m_slope_ladders.begin(), m_slope_ladders.end(), slope_ladder_comp);
		m_slope_ladders.pop_back();
		slope_ladder = m_slope_ladders.front();
		if (!slope_ladder->m_valid) return std::nullopt;
	}

	return slope_ladder;
}

bool IsolineSimplifier::step() {
//	check_valid();

	auto next = next_ladder();
	if (!next.has_value()) return false;
	auto slope_ladder = *next;

	m_changed_vertices.clear();

	for (int i = 0; i < slope_ladder->m_rungs.size(); i++) {
		collapse_edge(slope_ladder->m_rungs[i], slope_ladder->m_collapsed[i]);
	}

	slope_ladder->m_old = true;

	return true;
	// todo: update blocking numbers for slope ladders in vicinity of collapsed ladder, in terms of voronoi cells
}

void IsolineSimplifier::create_slope_ladder(Gt::Segment_2 seg, bool do_push_heap) {
	// Maybe make sure that the m_e_ladder and m_p_ladder maps never contains old ladders.
	if (m_e_ladder.contains(seg) && !m_e_ladder.at(seg)->m_old || m_e_ladder.contains(seg.opposite()) && !m_e_ladder.at(seg.opposite())->m_old) return;

	bool reversed = m_p_next.contains(seg.target()) && m_p_next.at(seg.target()) == seg.source();
	Gt::Point_2 s = reversed ? seg.target() : seg.source();
	Gt::Point_2 t = reversed ? seg.source() : seg.target();

	const auto search = [this](const Gt::Point_2& s, const Gt::Point_2& t, CGAL::Sign initial_dir, CGAL::Sign dir, std::shared_ptr<SlopeLadder> slope_ladder, const auto& search_f) {
		if (!(m_matching.contains(s) && m_matching.contains(t))) return;
		auto& s_matching = m_matching.at(s);
		auto& t_matching = m_matching.at(t);

		if (!(s_matching.contains(dir) && t_matching.contains(dir))) return;
		auto& s_m = s_matching.at(dir);
		auto& t_m = t_matching.at(dir);

		std::vector<Isoline<K>*> shared_isolines;
		for (const auto& [isoline_s_m, pts_s] : s_m) {
			for (const auto& [isoline_t_m, pts_t] : t_m) {
				if (isoline_s_m == isoline_t_m && !pts_s.empty() && !pts_t.empty()) {
					shared_isolines.push_back(isoline_s_m);
				}
			}
		}
		if (shared_isolines.empty()) return;
		for (const auto& shared_isoline : shared_isolines) {
			auto& sms = s_m.at(shared_isoline);
			auto& tms = t_m.at(shared_isoline);

			auto make_rung = [&](const Gt::Point_2& a, const Gt::Point_2& b) {
				m_e_ladder[Segment<K>(a, b)] = slope_ladder;
				if (initial_dir == CGAL::LEFT_TURN) {
					slope_ladder->m_rungs.emplace_front(a, b);
				} else {
					slope_ladder->m_rungs.emplace_back(a, b);
				}
				// Continue search in direction opposite of where we came from
				CGAL::Sign new_dir;
				bool found = false;
				for (CGAL::Sign possible_dir : {CGAL::LEFT_TURN, CGAL::RIGHT_TURN}) {
					if (m_matching.at(a).contains(possible_dir))
						for (const auto& [_, pts] : m_matching.at(a).at(possible_dir))
							for (const auto& pt : pts)
								if (pt == s) {
									new_dir = -possible_dir;
									found = true;
								}
				}
				assert(found);
				search_f(a, b, initial_dir, new_dir, slope_ladder, search_f);
			};

			for (const auto sp : sms) {
				for (const auto tp : tms) {
					if (sp == tp) {
						slope_ladder->m_cap[initial_dir] = sp;
						m_p_ladder[sp].push_back(slope_ladder);
						return;
					}
				}
			}
			for (const auto sp : sms) {
				for (const auto tp : tms) {
					if (m_p_next.contains(sp) && m_p_next.at(sp) == tp) {
						make_rung(sp, tp);
						return;
					} else if (m_p_prev.contains(sp) && m_p_prev.at(sp) == tp) {
						make_rung(sp, tp);
						return;
					}
				}
			}
		}
	    return;
	};

	// emplace_back on m_slope_ladders gives issues for some reason unknown to me
	std::shared_ptr<SlopeLadder> slope_ladder = std::make_shared<SlopeLadder>();
	slope_ladder->m_rungs.emplace_back(s, t);
	if (!reversed) {
		m_e_ladder[seg] = slope_ladder;
	} else {
		m_e_ladder[seg.opposite()] = slope_ladder;
	}

	search(s, t, CGAL::LEFT_TURN, CGAL::LEFT_TURN, slope_ladder, search);
	search(s, t, CGAL::RIGHT_TURN, CGAL::RIGHT_TURN, slope_ladder, search);

	for (const auto& rung : slope_ladder->m_rungs) {
		const auto& a = rung.source();
		const auto& b = rung.target();
		if (!m_p_prev.contains(a) || !m_p_next.contains(b) || !m_p_prev.contains(b) || !m_p_next.contains(a) || m_p_prev.at(a) == m_p_next.at(b)) {
			slope_ladder->m_valid = false;
		}
	}

	slope_ladder->compute_collapsed(m_p_prev, m_p_next);
	slope_ladder->compute_cost(m_p_prev, m_p_next);

	m_slope_ladders.push_back(slope_ladder);

	if (do_push_heap) {
		std::push_heap(m_slope_ladders.begin(), m_slope_ladders.end(), slope_ladder_comp);
	}
}

void IsolineSimplifier::initialize_slope_ladders() {
	for (const auto& isoline : m_simplified_isolines) {
		auto polyline = isoline.polyline();
		for (auto eit = polyline.edges_begin(); eit != polyline.edges_end(); ++eit) {
			create_slope_ladder(*eit, false);
		}
	}
	std::make_heap(m_slope_ladders.begin(), m_slope_ladders.end(), slope_ladder_comp);
}

void IsolineSimplifier::check_valid() {
	for (const auto& isoline : m_simplified_isolines) {
		for (auto pit = isoline.m_points.begin(); pit != isoline.m_points.end(); pit++) {
			if (pit != isoline.m_points.begin()) {
				auto prev = pit;
				prev--;
				assert(m_p_prev.at(*pit) == *prev);
			}
			if (pit != --isoline.m_points.end()) {
				auto next = pit;
				next++;
				assert(m_p_next.at(*pit) == *next);
			}
			assert(m_p_isoline.at(*pit) == &isoline);
		}
	}
}
}