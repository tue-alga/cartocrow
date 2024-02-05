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
	clean_isolines();
	initialize_sdg();
	initialize_point_data();
	m_separator = medial_axis_separator(m_delaunay, m_p_isoline, m_p_prev, m_p_next);
	m_matching = matching(m_delaunay, m_separator, m_p_prev, m_p_next, m_p_isoline);
	initialize_slope_ladders();
}

void IsolineSimplifier::initialize_point_data() {
	m_current_complexity = 0;
	for (auto& isoline : m_simplified_isolines) {
		for (auto pit = isoline.m_points.begin(); pit != isoline.m_points.end(); pit++) {
			++m_current_complexity;
			Gt::Point_2 p = *pit;
			if (m_p_isoline.contains(p)) {
				std::cerr << "Point " << p << " belongs to multiple isolines" << std::endl;
			}
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
	std::vector<Gt::Segment_2> segments;
	for (const auto& isoline : m_isolines) {
		auto polyline = isoline.polyline();
		std::copy(polyline.edges_begin(), polyline.edges_end(), std::back_inserter(segments));
	}
	m_delaunay.insert_segments(segments.begin(), segments.end());
}

void IsolineSimplifier::simplify(int target) {
	while (m_current_complexity > target) {
		if (m_current_complexity % 1000 == 0) {
			std::cout << "\r#Vertices: " << m_current_complexity << std::flush;
		}
		if (!step()) return;
		update_matching();
		update_ladders();
	}
}

void IsolineSimplifier::collapse_ladder(SlopeLadder& ladder) {
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
	  	m_deleted_points.push_back(p);

	    if (!m_delaunay.remove(vertex)) {
			std::cerr << "Likely point " << p << " is incident to a segment that has not yet been deleted." << std::endl;
		    throw std::runtime_error("Delaunay point vertex removal failed!");
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
			throw std::runtime_error("Delaunay segment vertex removal failed!");
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

	for (int i = 0; i < ladder.m_rungs.size(); i++) {
		auto edge = ladder.m_rungs.at(i);

		bool reversed = m_p_next.contains(edge.target()) && m_p_next.at(edge.target()) == edge.source();
		const Gt::Point_2& t = reversed ? edge.target() : edge.source();
		const Gt::Point_2& u = reversed ? edge.source() : edge.target();
		assert(m_p_next.at(t) == u && m_p_prev.at(u) == t);
		const Gt::Point_2& s = m_p_prev.at(t);
		const Gt::Point_2& v = m_p_next.at(u);
		const Gt::Segment_2 st = Gt::Segment_2(s, t);
		const Gt::Segment_2 uv = Gt::Segment_2(u, v);

		// Remove from Delaunay
		delaunay_remove_e(edge);
		delaunay_remove_e(st);
		delaunay_remove_e(uv);
		delaunay_remove_p(t);
		delaunay_remove_p(u);
	}

	for (int i = 0; i < ladder.m_rungs.size(); i++) {
		auto edge = ladder.m_rungs.at(i);
		auto new_point = ladder.m_collapsed.at(i);

		bool reversed = m_p_next.contains(edge.target()) && m_p_next.at(edge.target()) == edge.source();
		const Gt::Point_2& t = reversed ? edge.target() : edge.source();
		const Gt::Point_2& u = reversed ? edge.source() : edge.target();
		assert(m_p_next.at(t) == u && m_p_prev.at(u) == t);
		const Gt::Point_2& s = m_p_prev.at(t);
		const Gt::Point_2& v = m_p_next.at(u);

		// Insert into Delaunay
		delaunay_insert_e(s, new_point, m_p_vertex.at(s));
		auto seg_handle = delaunay_insert_e(new_point, v, m_p_vertex.at(v));
		delaunay_insert_p(new_point, seg_handle);
	}

	// Update the rest
	for (int i = 0; i < ladder.m_rungs.size(); i++) {
		--m_current_complexity;

		auto edge = ladder.m_rungs.at(i);
		auto new_point = ladder.m_collapsed.at(i);

		bool reversed = m_p_next.contains(edge.target()) && m_p_next.at(edge.target()) == edge.source();
		const Gt::Point_2& t = reversed ? edge.target() : edge.source();
		const Gt::Point_2& u = reversed ? edge.source() : edge.target();
		assert(m_p_next.at(t) == u && m_p_prev.at(u) == t);
		const Gt::Point_2& s = m_p_prev.at(t);
		const Gt::Point_2& v = m_p_next.at(u);
		const Gt::Segment_2 st = Gt::Segment_2(s, t);
		const Gt::Segment_2 uv = Gt::Segment_2(u, v);

		auto t_it = m_p_iterator.at(t);
		auto u_it = m_p_iterator.at(u);
		Isoline<K>* t_iso = m_p_isoline.at(t);
		Isoline<K>* u_iso = m_p_isoline.at(u);
		assert(t_iso == u_iso);

		// Remove points from isolines
		auto new_it = t_iso->m_points.insert(u_it, new_point);
		t_iso->m_points.erase(t_it);
		u_iso->m_points.erase(u_it);

		auto remove_ladder_p = [this](Gt::Point_2 point) {
			if (m_p_ladder.contains(point)) {
				for (const auto& l : m_p_ladder.at(point)) {
					l->m_old = true;
				}
				m_p_ladder.erase(point);
			}
		};

		// Update some ladder info
		remove_ladder_e(st);
		remove_ladder_e(uv);
		remove_ladder_e(st.opposite());
		remove_ladder_e(uv.opposite());

		remove_ladder_p(t);
		remove_ladder_p(u);

		// Update m_p_iterator
		m_p_iterator[new_point] = new_it;
		m_p_iterator.erase(t);
		m_p_iterator.erase(u);

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

		// Update intersects
		const auto update_intersects_e = [this](Gt::Segment_2 seg) {
			if (m_e_intersects.contains(seg)) {
				for (const auto& l : m_e_intersects.at(seg)) {
					l->m_intersects = false;
				}
			}
		};

		update_intersects_e(st);
		update_intersects_e(st.opposite());
		update_intersects_e(uv);
		update_intersects_e(uv.opposite());
		update_intersects_e(edge);
		update_intersects_e(edge.opposite());
	}
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
		if (!m_matching.contains(pt)) continue;
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
	std::vector<Gt::Segment_2> additional_segments;

	for (const auto& vh : m_changed_vertices) {
		const auto& site = vh->site();
		if (site.is_segment()) {
			const auto& seg = site.segment();
			remove_ladder_e(seg);
			remove_ladder_e(seg.opposite());
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
	if (m_slope_ladders.empty())
		return std::nullopt;

	// Note that m_slope_ladders is a heap
	auto current = m_slope_ladders.front();

	// Non-valid slope ladders have very high cost so this means no valid slope ladders are left
	if (!current->m_valid)
		return std::nullopt;

	std::vector<std::shared_ptr<SlopeLadder>> temp;
	std::optional<std::shared_ptr<SlopeLadder>> result;

	do {
		if (current->m_old) {
			std::pop_heap(m_slope_ladders.begin(), m_slope_ladders.end(), slope_ladder_comp);
			m_slope_ladders.pop_back();
		} else if (current->m_intersects) {
			temp.push_back(current);
			std::pop_heap(m_slope_ladders.begin(), m_slope_ladders.end(), slope_ladder_comp);
			m_slope_ladders.pop_back();
		} else if (IntersectionResult ir = check_ladder_intersections_Voronoi(*current)) {
			auto irv = *ir;
			// self-intersects
			if (holds_alternative<std::monostate>(irv)) {
			} else { // intersects another segment
				Gt::Segment_2 intersected = std::get<Gt::Segment_2>(irv);
				m_e_intersects[intersected].push_back(current);
			}
			current->m_intersects = true;
			temp.push_back(current);
			std::pop_heap(m_slope_ladders.begin(), m_slope_ladders.end(), slope_ladder_comp);
			m_slope_ladders.pop_back();
		} else {
			break;
		}
		current = m_slope_ladders.front();
		if (!current->m_valid) {
			break;
		}
	} while (true);

	if (!current->m_valid || current->m_old || check_ladder_intersections_Voronoi(*current)) {
		result = std::nullopt;
	} else {
		result = current;
	}

	if (result.has_value()) {
		std::pop_heap(m_slope_ladders.begin(), m_slope_ladders.end(), slope_ladder_comp);
		m_slope_ladders.pop_back();
	}

	for (const auto& ladder : temp) {
		m_slope_ladders.push_back(ladder);
		std::push_heap(m_slope_ladders.begin(), m_slope_ladders.end(), slope_ladder_comp);
	}

	return result;
}

bool IsolineSimplifier::step() {
//	check_valid();
	m_started = true;

	auto next = next_ladder();
	if (!next.has_value()) return false;
	auto slope_ladder = *next;

	m_changed_vertices.clear();
	m_deleted_points.clear();
	collapse_ladder(*slope_ladder);

	slope_ladder->m_old = true;

	return true;
}

void IsolineSimplifier::create_slope_ladder(Gt::Segment_2 seg, bool do_push_heap) {
	// Maybe make sure that the m_e_ladder and m_p_ladder maps never contains old ladders.
	if (m_e_ladder.contains(seg) &&
	        std::any_of(m_e_ladder.at(seg).begin(), m_e_ladder.at(seg).end(), [](const auto& l) { return !l->m_old; }) ||
	    m_e_ladder.contains(seg.opposite()) &&
			std::any_of(m_e_ladder.at(seg.opposite()).begin(), m_e_ladder.at(seg.opposite()).end(), [](const auto& l) { return !l->m_old; }))
	    return;

	bool reversed = m_p_next.contains(seg.target()) && m_p_next.at(seg.target()) == seg.source();
	Gt::Point_2 s = reversed ? seg.target() : seg.source();
	Gt::Point_2 t = reversed ? seg.source() : seg.target();

	const auto search = [this](const Gt::Point_2& s, const Gt::Point_2& t, CGAL::Sign initial_dir, CGAL::Sign dir, std::shared_ptr<SlopeLadder> slope_ladder, const auto& search_f) {
		bool reversed = m_p_next.contains(t) && m_p_next.at(t) == s;

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
//				if (m_e_ladder.contains(Segment<K>(a, b)) && !m_e_ladder.at(Segment<K>(a, b))->m_old || m_e_ladder.contains(Segment<K>(b, a)) && !m_e_ladder.at(Segment<K>(b, a))->m_old) {
//					std::cerr << "Encountered segment that is already part of a slope ladder" << std::endl;
//					std::cerr << "a: " << a << " b: " << b << std::endl;
//					std::cerr << "Encountered from s: " << s << " t: " << t << std::endl;
//					std::cerr << "Original slope ladder: ";
//					if (m_e_ladder.contains(Segment<K>(a, b))) {
//						std::cerr << m_e_ladder.at(Segment<K>(a, b))->m_rungs[0] << std::endl;
//					}
//					if (m_e_ladder.contains(Segment<K>(b, a))) {
//						std::cerr << m_e_ladder.at(Segment<K>(b, a))->m_rungs[0] << std::endl;
//					}
//				}
				m_e_ladder[Segment<K>(a, b)].push_back(slope_ladder);
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
				if (sp == s) {
					std::cerr << "Point " << s << " is matched to itself" << std::endl;
					continue;
				}
				for (const auto tp : tms) {
					if (tp == t) {
						std::cerr << "Point " << t << " is matched to itself" << std::endl;
						continue;
					}
					if (sp == tp) {
						// check whether sp (or tp) lies on the correct side of edge st
						if (reversed) {
							if (CGAL::orientation(t, s, sp) != dir) continue;
						} else {
							if (CGAL::orientation(s, t, sp) != dir) continue;
						}
						CGAL::Sign new_dir;
						for (CGAL::Sign possible_dir : {CGAL::LEFT_TURN, CGAL::RIGHT_TURN})
							if (m_matching.at(sp).contains(possible_dir))
								for (const auto& [_, pts] : m_matching.at(sp).at(possible_dir))
									for (const auto& pt : pts)
										if (pt == s) {
											new_dir = -possible_dir;
										}
						if (m_p_prev.contains(tp) && m_p_next.contains(tp)) {
							auto pr = m_p_prev.at(tp);
							auto ne = m_p_next.at(tp);
							if (CGAL::orientation(pr, tp, s) != -new_dir && CGAL::orientation(tp, ne, s) != -new_dir ||
							    CGAL::orientation(pr, tp, t) != -new_dir && CGAL::orientation(tp, ne, t) != -new_dir) continue;
						}

						slope_ladder->m_cap[initial_dir] = sp;
						m_p_ladder[sp].push_back(slope_ladder);
						return;
					}
				}
			}
			for (const auto sp : sms) {
				if (sp == s) {
					std::cerr << "Point " << s << " is matched to itself" << std::endl;
					continue;
				}
				for (const auto tp : tms) {
					if (tp == t) {
						std::cerr << "Point " << t << " is matched to itself" << std::endl;
						continue;
					}
					// check whether sp and tp lie on the correct side of edge st
					if (reversed) {
						if (CGAL::orientation(t, s, sp) != dir || CGAL::orientation(t, s, tp) != dir) continue;
					} else {
						if (CGAL::orientation(s, t, sp) != dir || CGAL::orientation(s, t, tp) != dir) continue;
					}
					CGAL::Sign new_dir;
					for (CGAL::Sign possible_dir : {CGAL::LEFT_TURN, CGAL::RIGHT_TURN})
						if (m_matching.at(sp).contains(possible_dir))
							for (const auto& [_, pts] : m_matching.at(sp).at(possible_dir))
								for (const auto& pt : pts)
									if (pt == s) {
										new_dir = -possible_dir;
									}
					bool new_reversed = m_p_next.contains(tp) && m_p_next.at(tp) == sp;
					if (new_reversed) {
						if (CGAL::orientation(tp, sp, s) != -new_dir || CGAL::orientation(tp, sp, t) != -new_dir) continue;
					} else {
						if (CGAL::orientation(sp, tp, s) != -new_dir || CGAL::orientation(sp, tp, t) != -new_dir) continue;
					}

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
		m_e_ladder[seg].push_back(slope_ladder);
	} else {
		m_e_ladder[seg.opposite()].push_back(slope_ladder);
	}

	search(s, t, CGAL::LEFT_TURN, CGAL::LEFT_TURN, slope_ladder, search);
	search(s, t, CGAL::RIGHT_TURN, CGAL::RIGHT_TURN, slope_ladder, search);

	for (const auto& rung : slope_ladder->m_rungs) {
		const auto& a = rung.source();
		const auto& b = rung.target();
		if (!m_p_prev.contains(a) || !m_p_next.contains(b) || !m_p_prev.contains(b) || !m_p_next.contains(a) || m_p_prev.at(a) == m_p_next.at(b) || m_p_next.at(a) == m_p_prev.at(b)) {
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

void IsolineSimplifier::clean_isolines() {
	for (int i = 0; i < m_simplified_isolines.size(); i++) {
		for (int j = 0; j < m_simplified_isolines.size(); j++) {
			if (i == j) continue;
			auto& isoline1 = m_simplified_isolines.at(i);
			auto& isoline2 = m_simplified_isolines.at(j);
			if (isoline1.m_points.back() == isoline2.m_points.front()) {
				isoline1.m_points.splice(isoline1.m_points.end(), isoline2.m_points);
			}
		}
	}

	erase_if(m_simplified_isolines, [](const auto& iso)  { return iso.m_points.empty(); });

	for (auto& isoline : m_simplified_isolines) {
		isoline.m_points.erase(std::unique(isoline.m_points.begin(), isoline.m_points.end()), isoline.m_points.end());
		if (isoline.m_points.front() == isoline.m_points.back()) {
			isoline.m_closed = true;
		}
	}

	for (auto& isoline : m_simplified_isolines) {
		if (isoline.m_closed && isoline.m_points.front() == isoline.m_points.back()) {
			isoline.m_points.pop_back();
		}
	}
}

bool IsolineSimplifier::check_ladder_intersections_naive(const SlopeLadder& ladder) const {
	assert(ladder.m_valid && !ladder.m_old);
	std::unordered_set<Gt::Segment_2> edges_to_skip;
	std::vector<Gt::Segment_2> new_edges;

	for (int i = 0; i < ladder.m_rungs.size(); i++) {
		const auto& rung = ladder.m_rungs.at(i);

		bool reversed = m_p_next.at(rung.target()) == rung.source();
		const auto& t = reversed ? rung.target() : rung.source();
		const auto& u = reversed ? rung.source() : rung.target();
		const auto& s = m_p_prev.at(t);
		const auto& v = m_p_next.at(u);
		const auto st = Gt::Segment_2(s, t);
		const auto tu = Gt::Segment_2(t, u);
		const auto uv = Gt::Segment_2(u, v);
		edges_to_skip.insert(st);
		edges_to_skip.insert(tu);
		edges_to_skip.insert(uv);

		const auto& p = ladder.m_collapsed.at(i);
		const auto sp = Gt::Segment_2(s, p);
		const auto pv = Gt::Segment_2(p, v);
		new_edges.push_back(sp);
		new_edges.push_back(pv);
	}

	for (const auto& isoline : m_simplified_isolines) {
		auto polyline = isoline.polyline();
		for (auto eit = polyline.edges_begin(); eit != polyline.edges_end(); eit++) {
			const auto& edge = *eit;
			if (edges_to_skip.contains(edge)) continue;
			for (int i = 0; i < ladder.m_rungs.size(); i++) {
				const auto& rung = ladder.m_rungs.at(i);
				const auto& p = ladder.m_collapsed.at(i);
				bool reversed = m_p_next.at(rung.target()) == rung.source();
				const auto& t = reversed ? rung.target() : rung.source();
				const auto& u = reversed ? rung.source() : rung.target();
				const auto& s = m_p_prev.at(t);
				const auto& v = m_p_next.at(u);
				const auto sp = Gt::Segment_2(s, p);
				const auto pv = Gt::Segment_2(p, v);
				auto spi = intersection(sp, edge);
				auto pvi = intersection(pv, edge);
				if (spi.has_value() && !(spi->type() == typeid(Gt::Point_2) && boost::get<Gt::Point_2>(*spi) == s) ||
				    pvi.has_value() && !(pvi->type() == typeid(Gt::Point_2) && boost::get<Gt::Point_2>(*pvi) == v))
					return true;
			}
		}
	}

	for (const auto& e1 : new_edges) {
		for (const auto& e2 : new_edges) {
			if (e1 == e2) continue;
			auto i = intersection(e1, e2);
			if (i.has_value()) {
				if (i->type() != typeid(Gt::Point_2))
					return true;
				auto p = boost::get<Gt::Point_2>(*i);
				if (p != e1.source() && p != e1.target())
					return true;
			}
		}
	}

	return false;
}

inline
    CGAL::Sign incircle(const SDG2& sdg, const SDG2::Site_2 &t1, const SDG2::Site_2 &t2,
             const SDG2::Site_2 &t3, const SDG2::Site_2 &q) {
	return sdg.geom_traits().vertex_conflict_2_object()(t1, t2, t3, q);
}

inline
    CGAL::Sign incircle(const SDG2& sdg, const SDG2::Site_2 &t1, const SDG2::Site_2 &t2,
             const SDG2::Site_2 &q) {
	return sdg.geom_traits().vertex_conflict_2_object()(t1, t2, q);
}

// https://github.com/CGAL/cgal/blob/96f698ca09b61b6ca7587d43b022a0db43519699/Segment_Delaunay_graph_2/include/CGAL/Segment_Delaunay_graph_2/Segment_Delaunay_graph_2_impl.h#L2320
CGAL::Sign
    incircle(const SDG2& sdg, const SDG2::Face_handle& f, const SDG2::Site_2& q)
{
	if ( !sdg.is_infinite(f) ) {
		return incircle(sdg,
		                f->vertex(0)->site(),
		                f->vertex(1)->site(),
		                f->vertex(2)->site(), q);
	}

	int inf_i(-1); // to avoid compiler warning
	for (int i = 0; i < 3; i++) {
		if ( sdg.is_infinite(f->vertex(i)) ) {
			inf_i = i;
			break;
		}
	}
	return incircle(sdg, f->vertex( SDG2::ccw(inf_i) )->site(),
	                f->vertex(  SDG2::cw(inf_i) )->site(), q );
}

Gt::Arrangement_type_2::result_type arrangement_type(const SDG2& sdg, const SDG2::Site_2& p, const SDG2::Site_2& q)
{
	typedef typename Gt::Arrangement_type_2  AT2;
	typedef typename AT2::result_type                 Arrangement_type;

	Arrangement_type res = sdg.geom_traits().arrangement_type_2_object()(p, q);

	if ( res == AT2::TOUCH_INTERIOR_12 || res == AT2::TOUCH_INTERIOR_21 ||
	    res == AT2::TOUCH_INTERIOR_11 || res == AT2::TOUCH_INTERIOR_22 ) {
		return AT2::DISJOINT;
	}
	if ( res == AT2::TOUCH_11 || res == AT2::TOUCH_12 ||
	    res == AT2::TOUCH_21 || res == AT2::TOUCH_22 ) {
		return AT2::DISJOINT;
	}

	return res;
}

std::optional<Gt::Segment_2> IsolineSimplifier::check_segment_intersections_Voronoi(const Gt::Segment_2 seg,
                                                            const SDG2::Vertex_handle endpoint_handle,
                                                            const std::unordered_set<SDG2::Vertex_handle>& allowed) const {
	auto t = SDG2::Site_2::construct_site_2(seg.source(), seg.target());

	auto check_intersections = [this, &allowed, &seg, &t, &endpoint_handle](SDG2::Vertex_handle vv) {
		if (!m_delaunay.is_infinite(vv) && vv->is_segment()) {
			bool intersects = arrangement_type(m_delaunay, t, vv->site()) == Gt::Arrangement_type_2::result_type::CROSSING;
			if (intersects) {
				return true;
			}
//			auto inter = CGAL::intersection(vv->site().segment(), seg);
//			if (inter.has_value()) {
//				auto i = *inter;
//				if (i.type() != typeid(Gt::Point_2) || boost::get<Gt::Point_2>(i) != endpoint_handle->site().point()) {
//					return true;
//				}
//			}

		}
		return false;
	};

	auto vc_start = m_delaunay.incident_vertices(endpoint_handle);
	auto vc = vc_start;
	do {
		SDG2::Vertex_handle vv(vc);
		if (m_delaunay.is_infinite(vv)) {
			++vc;
			continue;
		}

		if (check_intersections(vv) && !allowed.contains(vv)) return vv->site().segment();
		++vc;
	} while (vc != vc_start);

	// First, find one face that is in conflict with seg (i.e. seg is close to corresponding vertex of Voronoi diagram)
	SDG2::Face_circulator fc_start = m_delaunay.incident_faces(endpoint_handle);
	SDG2::Face_circulator fc = fc_start;
	SDG2::Face_handle start_f;
	CGAL::Sign s;

	do {
		SDG2::Face_handle f(fc);
		s = incircle(m_delaunay, f, t);

		if (s == CGAL::NEGATIVE) {
			start_f = f;
			break;
		}
		++fc;
	} while (fc != fc_start);

	assert(s == CGAL::NEGATIVE);

	std::unordered_set<SDG2::Face_handle> visited;
	std::stack<SDG2::Face_handle> face_stack;
	std::unordered_set<SDG2::Face_handle> positive;
	face_stack.push(start_f);

	while (!face_stack.empty()) {
		const auto curr_f = face_stack.top();
		face_stack.pop();

		// Already visited, so skip
		if (visited.contains(curr_f)) {
			continue;
		}
		visited.insert(curr_f);

		for (int i = 0; i < 3; i++) {
			auto n = curr_f->neighbor(i);
			if (visited.contains(n)) continue;

			for (int j = 0; j < 3; j++) {
				auto vv = n->vertex(j);
				if (check_intersections(vv)) {
					if (!allowed.contains(vv))
						return vv->site().segment();
					else {
						auto vfc_start = m_delaunay.incident_faces(vv);
						auto vfc = vfc_start;
						do {
							SDG2::Face_handle f(vfc);
							face_stack.push(f);
							++vfc;
						} while (vfc != vfc_start);
					}
				}
			}

			s = incircle(m_delaunay, n, t);

			if (positive.contains(curr_f) && s == CGAL::POSITIVE) continue;

			face_stack.push(n);
			if (s == CGAL::POSITIVE) {
				positive.insert(n);
			}
		}
	}

	// If we are done and haven't found intersections then there are none.
	return std::nullopt;
}

IntersectionResult IsolineSimplifier::check_ladder_intersections_Voronoi(const SlopeLadder& ladder) const {
	assert(ladder.m_valid && !ladder.m_old);
	std::unordered_set<SDG2::Vertex_handle> edges_to_skip;
	std::vector<Gt::Segment_2> new_edges;

	for (int i = 0; i < ladder.m_rungs.size(); i++) {
		const auto& rung = ladder.m_rungs.at(i);

		bool reversed = m_p_next.contains(rung.target()) && m_p_next.at(rung.target()) == rung.source();
		const auto& t = reversed ? rung.target() : rung.source();
		const auto& u = reversed ? rung.source() : rung.target();
		const auto& s = m_p_prev.at(t);
		const auto& v = m_p_next.at(u);
		const auto st = Gt::Segment_2(s, t);
		const auto tu = Gt::Segment_2(t, u);
		const auto uv = Gt::Segment_2(u, v);
		edges_to_skip.insert(m_e_vertex.at(st));
		edges_to_skip.insert(m_e_vertex.at(tu));
		edges_to_skip.insert(m_e_vertex.at(uv));

		const auto& p = ladder.m_collapsed.at(i);
		const auto sp = Gt::Segment_2(s, p);
		const auto pv = Gt::Segment_2(p, v);
		new_edges.push_back(sp);
		new_edges.push_back(pv);
	}

	for (int i = 0; i < ladder.m_rungs.size(); i++) {
		const auto& rung = ladder.m_rungs.at(i);

		bool reversed = m_p_next.contains(rung.target()) && m_p_next.at(rung.target()) == rung.source();
		const auto& t = reversed ? rung.target() : rung.source();
		const auto& u = reversed ? rung.source() : rung.target();
		const auto& s = m_p_prev.at(t);
		const auto& v = m_p_next.at(u);

		const auto& p = ladder.m_collapsed.at(i);
		const auto sp = Gt::Segment_2(s, p);
		const auto pv = Gt::Segment_2(p, v);

		auto spi = check_segment_intersections_Voronoi(sp, m_p_vertex.at(s), edges_to_skip);
		if (spi.has_value()) return spi;
		auto pvi = check_segment_intersections_Voronoi(pv, m_p_vertex.at(v), edges_to_skip);
		if (pvi.has_value()) return pvi;
	}

	for (const auto& e1 : new_edges) {
		for (const auto& e2 : new_edges) {
			if (e1 == e2) continue;
			auto i = intersection(e1, e2);
			if (i.has_value()) {
				if (i->type() != typeid(Gt::Point_2))
					return std::monostate();
				auto p = boost::get<Gt::Point_2>(*i);
				if (p != e1.source() && p != e1.target())
					return std::monostate();
			}
		}
	}

	return std::nullopt;
}

void IsolineSimplifier::remove_ladder_e(Gt::Segment_2 seg) {
	if (m_e_ladder.contains(seg)) {
		for (const auto& ladder : m_e_ladder.at(seg)) {
			ladder->m_old = true;
		}
		m_e_ladder.erase(seg);
	}
}
}