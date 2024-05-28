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

#include "isoline_simplifier.h"
#include "collapse.h"
#include "symmetric_difference.h"
#include "voronoi_helpers.h"
#include <CGAL/Arr_conic_traits_2.h>
#include <CGAL/CORE_algebraic_number_traits.h>
#include <CGAL/Cartesian.h>

#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Constrained_triangulation_plus_2.h>
#include <CGAL/Polyline_simplification_2/simplify.h>

#include <utility>
#include "ipe_bezier_wrapper.h"

namespace cartocrow::isoline_simplification {
namespace PS = CGAL::Polyline_simplification_2;
typedef PS::Stop_below_count_threshold Stop;
typedef PS::Hybrid_squared_distance_cost<K::FT>      Cost;

typedef PS::Vertex_base_2<K> Vb;
typedef CGAL::Constrained_triangulation_face_base_2<K> Fb;
typedef CGAL::Triangulation_data_structure_2<Vb, Fb> TDS;
typedef CGAL::Constrained_Delaunay_triangulation_2<K, TDS, CGAL::Exact_predicates_tag> CDT;
typedef CGAL::Constrained_triangulation_plus_2<CDT>     CT;

IsolineSimplifier::IsolineSimplifier(std::vector<Isoline<K>> isolines, std::shared_ptr<LadderCollapse> collapse,
                                     double angle_filter, double alignment_filter):
      m_isolines(std::move(isolines)), m_angle_filter(angle_filter), m_alignment_filter(alignment_filter), m_collapse_ladder(std::move(collapse)) {
	clean_isolines();
	m_simplified_isolines = m_isolines;
	initialize_sdg();
	initialize_point_data();
	m_separator = medial_axis_separator(m_delaunay, m_p_isoline, m_p_prev, m_p_next);
	m_matching = matching(m_delaunay, m_separator, m_p_prev, m_p_next, m_p_isoline, m_p_vertex, m_angle_filter, m_alignment_filter);
	initialize_slope_ladders();
}

void IsolineSimplifier::initialize_point_data() {
	m_current_complexity = 0;
	for (auto& isoline : m_simplified_isolines) {
		for (auto pit = isoline.m_points.begin(); pit != isoline.m_points.end(); pit++) {
			++m_current_complexity;
			Point<K> p = *pit;
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
}

void IsolineSimplifier::initialize_sdg() {
	m_delaunay.clear();
	m_e_vertex.clear();
	m_p_vertex.clear();
	std::vector<Segment<K>> segments;
	for (const auto& isoline : m_simplified_isolines) {
		auto polyline = isoline.polyline();
		std::copy(polyline.edges_begin(), polyline.edges_end(), std::back_inserter(segments));
	}
	m_delaunay.insert_segments(segments.begin(), segments.end());

	for (auto vit = m_delaunay.finite_vertices_begin(); vit != m_delaunay.finite_vertices_end(); vit++) {
		auto site = vit->site();
		if (site.is_point()) {
			m_p_vertex[site.point()] = vit;
		} else {
			m_e_vertex[site.segment()] = vit;
		}
	}
}

bool IsolineSimplifier::simplify(int target, bool debug) {
	while (m_current_complexity > target) {
		if (debug && m_current_complexity % 1000 == 0) {
			std::cout << "\r#Vertices: " << m_current_complexity << std::flush;
		}
		if (!step()) return false;
		update_matching();
		update_ladders();
	}
	return true;
}

bool IsolineSimplifier::dyken_simplify(int target, double sep_dist) {
	m_started = true;
	int start_complexity = m_current_complexity;

	CT ct;

	std::vector<CT::Constraint_id> ids;

	for (const auto& isoline : m_simplified_isolines) {
		if (isoline.m_closed) {
			CT::Constraint_id id = ct.insert_constraint(isoline.polygon());
			ids.push_back(id);
		} else {
			CT::Constraint_id id = ct.insert_constraint(isoline.m_points);
			ids.push_back(id);
		}
	}

	std::vector<Isoline<K>> result(m_simplified_isolines.size());

	m_current_complexity -= PS::simplify(ct, Cost(sep_dist), Stop(target));
	for(auto cit = ct.constraints_begin(); cit != ct.constraints_end(); ++cit) {
		CT::Constraint_id id = *cit;
		auto i = std::distance(ids.begin(), std::find(ids.begin(), ids.end(), id));

		std::vector<Point<K>> simplified_points;
		for (auto vit = ct.points_in_constraint_begin(*cit); vit != ct.points_in_constraint_end(*cit); ++vit) {
			simplified_points.push_back(*vit);
		}
		if (m_simplified_isolines[i].m_closed) {
			simplified_points.pop_back();
		}
		result[i] = Isoline<K>(simplified_points, m_simplified_isolines[i].m_closed);
	}

	m_simplified_isolines = result;

	return start_complexity != m_current_complexity;
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

	auto delaunay_remove_p = [&insert_adj, this](const Point<K>& p) {
	 	auto vertex = m_p_vertex.at(p);
	 	insert_adj(vertex);
	  	m_changed_vertices.erase(vertex);
	  	m_deleted_points.push_back(p);

	    if (!m_delaunay.remove(vertex)) {
			throw std::runtime_error("Point removal failed\nThe point is likely incident to a segment that has not yet been deleted.");
	    }
	};

	auto delaunay_remove_e = [&insert_adj, this](const Segment<K>& s) {
		auto seg = s;
		if (!m_e_vertex.contains(s)) {
			if (m_e_vertex.contains(s.opposite())) {
				seg = s.opposite();
			} else {
				std::cerr << "\nSegment: " << s << std::endl;
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

	auto delaunay_insert_p = [this, &insert_adj](const Point<K>& p, const SDG2::Vertex_handle near) {
	  	auto handle = m_delaunay.insert(p, near);
	    m_changed_vertices.insert(handle);
	  	m_p_vertex[p] = handle;
	  	insert_adj(handle);
		return handle;
	};

	auto delaunay_insert_e = [this, &insert_adj](const Point<K>& p1, const Point<K>& p2, const SDG2::Vertex_handle near) {
	  	auto handle = m_delaunay.insert(p1, p2, near);
	  	m_e_vertex[Segment<K>(p1, p2)] = handle;
		m_changed_vertices.insert(handle);
	  	insert_adj(handle);
		return handle;
	};

	// Remove from Delaunay
	for (auto edge : ladder.m_rungs) {
		bool reversed =
		    m_p_next.contains(edge.target()) && m_p_next.at(edge.target()) == edge.source();
		const Point<K>& t = reversed ? edge.target() : edge.source();
		const Point<K>& u = reversed ? edge.source() : edge.target();
		const Point<K>& s = m_p_prev.at(t);
		const Point<K>& v = m_p_next.at(u);
		const Segment<K> st = Segment<K>(s, t);
		const Segment<K> uv = Segment<K>(u, v);

		delaunay_remove_e(edge);
		delaunay_remove_e(st);
		delaunay_remove_p(t);
		delaunay_remove_e(uv);
		delaunay_remove_p(u);
	}

	// Insert into Delaunay
	for (int i = 0; i < ladder.m_rungs.size(); i++) {
		auto edge = ladder.m_rungs.at(i);
		auto new_point = ladder.m_collapsed.at(i);

		bool reversed = m_p_next.contains(edge.target()) && m_p_next.at(edge.target()) == edge.source();
		const Point<K>& t = reversed ? edge.target() : edge.source();
		const Point<K>& u = reversed ? edge.source() : edge.target();
		assert(m_p_next.at(t) == u && m_p_prev.at(u) == t);
		const Point<K>& s = m_p_prev.at(t);
		const Point<K>& v = m_p_next.at(u);

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
		const Point<K> t = reversed ? edge.target() : edge.source();
		const Point<K> u = reversed ? edge.source() : edge.target();
		assert(m_p_next.at(t) == u && m_p_prev.at(u) == t);
		const Point<K> s = m_p_prev.at(t);
		const Point<K> v = m_p_next.at(u);
		const Segment<K> st = Segment<K>(s, t);
		const Segment<K> uv = Segment<K>(u, v);

		auto t_it = m_p_iterator.at(t);
		auto u_it = m_p_iterator.at(u);
		Isoline<K>* t_iso = m_p_isoline.at(t);
		Isoline<K>* u_iso = m_p_isoline.at(u);
		assert(t_iso == u_iso);

		// Remove points from isolines
		auto new_it = t_iso->m_points.insert(u_it, new_point);
		t_iso->m_points.erase(t_it);
		u_iso->m_points.erase(u_it);

		auto remove_ladder_p = [this](Point<K> point) {
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
		remove_ladder_e(edge);
		remove_ladder_e(edge.opposite());

		remove_ladder_p(t);
		remove_ladder_p(u);

		// Update m_p_iterator
		m_p_iterator.erase(t);
		m_p_iterator.erase(u);
		m_p_iterator[new_point] = new_it;

		// Update m_p_isoline
		m_p_isoline.erase(t);
		m_p_isoline.erase(u);
		m_p_isoline[new_point] = t_iso;

		// Update prev and next
		m_p_prev.erase(t);
		m_p_next.erase(t);
		m_p_prev.erase(u);
		m_p_next.erase(u);
		m_p_prev[v] = new_point;
		m_p_next[s] = new_point;
		if (m_p_prev.contains(new_point)) {
			std::cerr << "Collapsed to existing point!" << std::endl;
		}
		m_p_prev[new_point] = s;
		if (m_p_next.contains(new_point)) {
			std::cerr << "Collapsed to existing point!" << std::endl;
		}
		m_p_next[new_point] = v;

		// Update intersects
		const auto update_intersects_e = [this](Segment<K> seg) {
			if (m_e_intersects.contains(seg)) {
				for (const auto& l : m_e_intersects.at(seg)) {
					if (!l->m_old) {
						l->m_intersects = false;
						l->compute_cost(m_p_prev, m_p_next);
						m_slope_ladders.increase(m_ladder_heap_handle.at(l));
					}
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
	std::unordered_set<Point<K>> updated_points;
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
		// The below should not be necessary, but this prevents a rare crash on large-western-island under
		// minimize symmetric difference collapse.
		if (m_matching.contains(p)) {
			for (auto& [sign, mi] : m_matching.at(p)) {
				for (auto& [iso, pts] : mi) {
					for (auto& pt : pts) {
						updated_points.insert(pt);
					}
				}
			}
		}
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

	std::vector<Point<K>> modified_matchings;
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
				auto site_2 = target->site();
				auto iso_2 = m_p_isoline.at(point_of_site(site_2));
				if (iso_1 == iso_2) continue;
				create_matching(m_delaunay, edge, m_matching, m_p_prev, m_p_next, m_p_isoline, m_p_vertex, m_angle_filter, m_alignment_filter);
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
				std::sort(pts.begin(), pts.end());
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
	std::vector<Segment<K>> additional_segments;

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

	auto check_subset = [](const std::shared_ptr<SlopeLadder>& sub, const std::shared_ptr<SlopeLadder>& super) {
		for (auto sign : {CGAL::LEFT_TURN, CGAL::RIGHT_TURN}) {
			if (sub->m_cap.contains(sign)) {
				if (!super->m_cap.contains(sign))
					return false;
				if (super->m_cap.at(sign) != sub->m_cap.at(sign))
					return false;
			}
		}
		for (const auto& rung : sub->m_rungs) {
			auto contained = std::find_if(super->m_rungs.begin(), super->m_rungs.end(), [&rung](const Segment<K>& other_rung) {
				return rung == other_rung || rung == other_rung.opposite();
			}) != super->m_rungs.end();
			if (!contained) return false;
		}
		return true;
	};

	auto remove_subset_ladders = [this, &check_subset](Segment<K> seg) {
		if (!m_e_ladder.contains(seg)) return;
		for (const auto& ladder : m_e_ladder.at(seg)) {
			if (ladder->m_old) continue;
			std::unordered_set<std::shared_ptr<SlopeLadder>> other_ladders;

			for (const auto& rung : ladder->m_rungs) {
				if (m_e_ladder.contains(rung)) {
					for (const auto& other_ladder : m_e_ladder.at(rung)) {
						if (other_ladder->m_old) continue;
						if (other_ladder != ladder) {
							other_ladders.insert(other_ladder);
						}
					}
				}
				if (m_e_ladder.contains(rung.opposite())) {
					for (const auto& other_ladder : m_e_ladder.at(rung.opposite())) {
						if (other_ladder->m_old) continue;
						if (other_ladder != ladder) {
							other_ladders.insert(other_ladder);
						}
					}
				}
			}

			for (const auto& other_ladder : other_ladders) {
				if (check_subset(other_ladder, ladder)) {
					other_ladder->m_old = true;
				}
			}
		}
	};

	for (const auto& vh : m_changed_vertices) {
		const auto& site = vh->site();
		if (site.is_point()) {
			auto p = site.point();
			if (m_p_prev.contains(p)) {
				Segment<K> seg(m_p_prev.at(p), p);
				create_slope_ladder(seg);
				remove_subset_ladders(seg);
				remove_subset_ladders(seg.opposite());
			}
			if (m_p_next.contains(p)) {
				Segment<K> seg(p, m_p_next.at(p));
				create_slope_ladder(seg);
				remove_subset_ladders(seg);
				remove_subset_ladders(seg.opposite());
			}
		} else {
			create_slope_ladder(site.segment());
			remove_subset_ladders(site.segment());
			remove_subset_ladders(site.segment().opposite());
		}
	}

	for (const auto& seg : additional_segments) {
		create_slope_ladder(seg);
		remove_subset_ladders(seg);
		remove_subset_ladders(seg.opposite());
	}
}

// same as next_ladder() but does not pop the next ladder from the heap.
std::optional<std::shared_ptr<SlopeLadder>> IsolineSimplifier::get_next_ladder() {
	if (m_slope_ladders.empty())
		return std::nullopt;

	// Note that m_slope_ladders is a heap
	auto current = m_slope_ladders.top();

	// Non-valid slope ladders have very high cost so this means no valid slope ladders are left
	if (!current->m_valid)
		return std::nullopt;

	std::vector<std::shared_ptr<SlopeLadder>> temp;
	std::optional<std::shared_ptr<SlopeLadder>> result;

	bool found = false;

	do {
		if (current->m_old) {
			m_ladder_heap_handle.erase(current);
			m_slope_ladders.pop();
		} else if (current->m_intersects) {
			temp.push_back(current);
			m_ladder_heap_handle.erase(current);
			m_slope_ladders.pop();
		} else if (IntersectionResult ir = check_ladder_intersections_Voronoi(*current)) {
			auto irv = *ir;
			// self-intersects
			if (holds_alternative<std::monostate>(irv)) {
			} else { // intersects another segment
				Segment<K> intersected = std::get<Segment<K>>(irv);
				m_e_intersects[intersected].push_back(current);
			}
			current->m_intersects = true;
			temp.push_back(current);
			m_ladder_heap_handle.erase(current);
			m_slope_ladders.pop();
		}
		else if (check_ladder_collapse_topology(*current)) {
			temp.push_back(current);
			m_ladder_heap_handle.erase(current);
			m_slope_ladders.pop();
		}
		else {
			found = true;
			break;
		}
		current = m_slope_ladders.top();
		if (!current->m_valid) {
			break;
		}
	} while (!m_slope_ladders.empty());

	if (!found) {
		result = std::nullopt;
	} else {
		result = current;
	}

	for (const auto& ladder : temp) {
		m_ladder_heap_handle[ladder] = m_slope_ladders.push(ladder);
	}

	return result;
}

std::optional<std::shared_ptr<SlopeLadder>> IsolineSimplifier::next_ladder() {
	if (m_slope_ladders.empty())
		return std::nullopt;

	// Note that m_slope_ladders is a heap
	auto current = m_slope_ladders.top();

	// Non-valid slope ladders have very high cost so this means no valid slope ladders are left
	if (!current->m_valid)
		return std::nullopt;

	std::vector<std::shared_ptr<SlopeLadder>> temp;
	std::optional<std::shared_ptr<SlopeLadder>> result;

	bool found = false;

	do {
		bool old_but_not_correctly_updated = false;

		for (const auto& rung : current->m_rungs) {
			old_but_not_correctly_updated |= !m_p_iterator.contains(rung.source());
			old_but_not_correctly_updated |= !m_p_iterator.contains(rung.target());
		}

		if (current->m_old) {
			m_ladder_heap_handle.erase(current);
			m_slope_ladders.pop();
		} else if (old_but_not_correctly_updated) {
			std::cerr << "Incorrectly updated" << std::endl;
			m_ladder_heap_handle.erase(current);
			m_slope_ladders.pop();
		}
		else if (current->m_intersects) {
			temp.push_back(current);
			m_ladder_heap_handle.erase(current);
			m_slope_ladders.pop();
		}
		else if (IntersectionResult ir = check_ladder_intersections_Voronoi(*current)) {
			auto irv = *ir;
			// self-intersects
			if (holds_alternative<std::monostate>(irv)) {
			} else { // intersects another segment
				Segment<K> intersected = std::get<Segment<K>>(irv);
				m_e_intersects[intersected].push_back(current);
			}
			current->m_intersects = true;
			current->m_cost = std::numeric_limits<double>::infinity();
			temp.push_back(current);
			m_ladder_heap_handle.erase(current);
			m_slope_ladders.pop();
		}
		else if (check_ladder_collapse_topology(*current)) {
			temp.push_back(current);
			m_ladder_heap_handle.erase(current);
			m_slope_ladders.pop();
		}
		else {
			found = true;
			break;
		}
		current = m_slope_ladders.top();
		if (!current->m_valid) {
			break;
		}
	} while (!m_slope_ladders.empty());

	if (!found) {
		result = std::nullopt;
	} else {
		result = current;
	}

	if (result.has_value()) {
		m_ladder_heap_handle.erase(current);
		m_slope_ladders.pop();
	}

	for (const auto& ladder : temp) {
		m_ladder_heap_handle[ladder] = m_slope_ladders.push(ladder);
	}

	return result;
}

bool IsolineSimplifier::step() {
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

void IsolineSimplifier::create_slope_ladder(Segment<K> seg) {
	if (m_e_ladder.contains(seg) &&
	        std::any_of(m_e_ladder.at(seg).begin(), m_e_ladder.at(seg).end(), [](const auto& l) { return !l->m_old; }) ||
	    m_e_ladder.contains(seg.opposite()) &&
			std::any_of(m_e_ladder.at(seg.opposite()).begin(), m_e_ladder.at(seg.opposite()).end(), [](const auto& l) { return !l->m_old; }))
	    return;

	bool reversed = m_p_next.contains(seg.target()) && m_p_next.at(seg.target()) == seg.source();
	Point<K> s = reversed ? seg.target() : seg.source();
	Point<K> t = reversed ? seg.source() : seg.target();

	const auto search = [this](const Point<K>& s, const Point<K>& t, CGAL::Sign initial_dir, CGAL::Sign dir, std::shared_ptr<SlopeLadder> slope_ladder, const auto& search_f) {
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

			auto make_rung = [&](const Point<K>& a, const Point<K>& b) {
				m_e_ladder[Segment<K>(a, b)].push_back(slope_ladder);
				if (initial_dir == CGAL::LEFT_TURN) {
					slope_ladder->m_rungs.emplace_front(a, b);
				} else {
					slope_ladder->m_rungs.emplace_back(a, b);
				}
				// Continue search in direction opposite of where we came from
				CGAL::Sign new_dir;
				for (CGAL::Sign possible_dir : {CGAL::LEFT_TURN, CGAL::RIGHT_TURN}) {
					if (m_matching.at(a).contains(possible_dir))
						for (const auto& [_, pts] : m_matching.at(a).at(possible_dir))
							for (const auto& pt : pts)
								if (pt == s) {
									new_dir = -possible_dir;
								}
				}
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

	(*m_collapse_ladder)(*slope_ladder, m_p_prev, m_p_next);
	slope_ladder->compute_cost(m_p_prev, m_p_next);

	m_ladder_heap_handle[slope_ladder] = m_slope_ladders.push(slope_ladder);
}

void IsolineSimplifier::initialize_slope_ladders() {
	for (const auto& isoline : m_simplified_isolines) {
		auto polyline = isoline.polyline();
		for (auto eit = polyline.edges_begin(); eit != polyline.edges_end(); ++eit) {
			create_slope_ladder(*eit);
		}
	}
}

void IsolineSimplifier::clean_isolines() {
	for (int i = 0; i < m_isolines.size(); i++) {
		for (int j = 0; j < m_isolines.size(); j++) {
			if (i == j) continue;
			auto& isoline1 = m_isolines.at(i);
			auto& isoline2 = m_isolines.at(j);
			if (isoline1.m_points.back() == isoline2.m_points.front()) {
				isoline1.m_points.splice(isoline1.m_points.end(), isoline2.m_points);
			}
		}
	}

	erase_if(m_isolines, [](const auto& iso)  { return iso.m_points.empty(); });

	for (auto& isoline : m_isolines) {
		isoline.m_points.erase(std::unique(isoline.m_points.begin(), isoline.m_points.end()), isoline.m_points.end());
		if (isoline.m_points.front() == isoline.m_points.back()) {
			isoline.m_closed = true;
		}
	}

	for (auto& isoline : m_isolines) {
		if (isoline.m_closed && isoline.m_points.front() == isoline.m_points.back()) {
			isoline.m_points.pop_back();
		}
	}
}

bool IsolineSimplifier::check_ladder_intersections_naive(const SlopeLadder& ladder) const {
	assert(ladder.m_valid && !ladder.m_old);
	std::unordered_set<Segment<K>> edges_to_skip;
	std::vector<Segment<K>> new_edges;

	for (int i = 0; i < ladder.m_rungs.size(); i++) {
		const auto& rung = ladder.m_rungs.at(i);

		bool reversed = m_p_next.at(rung.target()) == rung.source();
		const auto& t = reversed ? rung.target() : rung.source();
		const auto& u = reversed ? rung.source() : rung.target();
		const auto& s = m_p_prev.at(t);
		const auto& v = m_p_next.at(u);
		const auto st = Segment<K>(s, t);
		const auto tu = Segment<K>(t, u);
		const auto uv = Segment<K>(u, v);
		edges_to_skip.insert(st);
		edges_to_skip.insert(tu);
		edges_to_skip.insert(uv);

		const auto& p = ladder.m_collapsed.at(i);
		const auto sp = Segment<K>(s, p);
		const auto pv = Segment<K>(p, v);
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
				const auto sp = Segment<K>(s, p);
				const auto pv = Segment<K>(p, v);
				auto spi = intersection(sp, edge);
				auto pvi = intersection(pv, edge);
				if (spi.has_value() && !(spi->type() == typeid(Point<K>) && boost::get<Point<K>>(*spi) == s) ||
				    pvi.has_value() && !(pvi->type() == typeid(Point<K>) && boost::get<Point<K>>(*pvi) == v))
					return true;
			}
		}
	}

	for (const auto& e1 : new_edges) {
		for (const auto& e2 : new_edges) {
			if (e1 == e2) continue;
			auto i = intersection(e1, e2);
			if (i.has_value()) {
				if (i->type() != typeid(Point<K>))
					return true;
				auto p = boost::get<Point<K>>(*i);
				if (p != e1.source() && p != e1.target())
					return true;
			}
		}
	}

	return false;
}

IntersectionResult IsolineSimplifier::check_ladder_intersections_Voronoi(const SlopeLadder& ladder) {
	assert(ladder.m_valid && !ladder.m_old);
	std::unordered_set<SDG2::Vertex_handle> edges_to_skip;
	std::vector<Segment<K>> new_edges;

	for (int i = 0; i < ladder.m_rungs.size(); i++) {
		const auto& rung = ladder.m_rungs.at(i);

		bool reversed = m_p_next.contains(rung.target()) && m_p_next.at(rung.target()) == rung.source();
		const auto& t = reversed ? rung.target() : rung.source();
		const auto& u = reversed ? rung.source() : rung.target();
		const auto& s = m_p_prev.at(t);
		const auto& v = m_p_next.at(u);
		const auto st = Segment<K>(s, t);
		const auto tu = Segment<K>(t, u);
		const auto uv = Segment<K>(u, v);
		edges_to_skip.insert(m_e_vertex.at(st));
		edges_to_skip.insert(m_e_vertex.at(tu));
		edges_to_skip.insert(m_e_vertex.at(uv));

		const auto& p = ladder.m_collapsed.at(i);
		const auto sp = Segment<K>(s, p);
		const auto pv = Segment<K>(p, v);
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
		const auto sp = Segment<K>(s, p);
		const auto pv = Segment<K>(p, v);
		Segment<K> st(s, t);
		Segment<K> uv(u, v);

		std::optional<SDG2::Vertex_handle> st_coll;
		if (squared_distance(st.supporting_line(), p) < 1E-9) {
			st_coll = m_e_vertex.at(st);
		}
		auto spi = check_segment_intersections_Voronoi(m_delaunay, sp, m_p_vertex.at(s), edges_to_skip, st_coll);
		if (spi.has_value()) return spi;

		std::optional<SDG2::Vertex_handle> uv_coll;
		if (squared_distance(uv.supporting_line(), p) < 1E-9) {
			uv_coll = m_e_vertex.at(uv);
		}
		auto pvi = check_segment_intersections_Voronoi(m_delaunay, pv, m_p_vertex.at(v), edges_to_skip, uv_coll);
		if (pvi.has_value()) return pvi;
	}

	for (const auto& e1 : new_edges) {
		for (const auto& e2 : new_edges) {
			if (e1 == e2) continue;
			auto i = intersection(e1, e2);
			if (i.has_value()) {
				if (i->type() != typeid(Point<K>))
					return std::monostate();
				auto p = boost::get<Point<K>>(*i);
				if (p != e1.source() && p != e1.target())
					return std::monostate();
			}
		}
	}

	return std::nullopt;
}

void IsolineSimplifier::remove_ladder_e(Segment<K> seg) {
	if (m_e_ladder.contains(seg)) {
		for (const auto& ladder : m_e_ladder.at(seg)) {
			ladder->m_old = true;
		}
		m_e_ladder.erase(seg);
	}
}

std::vector<Point<K>> intersections_primal(Segment<K> seg, const CGAL::Object& o) {
	Segment<K> s;
	Line<K> l;
	Ray<K> r;
	CGAL::Parabola_segment_2<Gt> ps;
	std::vector<Point<K>> intersections;

	if (CGAL::assign(s, o)) {
		auto inters = CGAL::intersection(s, seg);
		if (inters.has_value()) {
			auto v = *inters;
			if (auto pp = boost::get<Point<K>>(&v)) {
				intersections.push_back(*pp);
			}
		}
		return intersections;
	} else if (CGAL::assign(l, o)) {
		auto inters = CGAL::intersection(l, seg);
		if (inters.has_value()) {
			auto v = *inters;
			if (auto pp = boost::get<Point<K>>(&v)) {
				intersections.push_back(*pp);
			}
		}
		return intersections;
	} else if (CGAL::assign(r, o)) {
		auto inters = CGAL::intersection(r, seg);
		if (inters.has_value()) {
			auto v = *inters;
			if (auto pp = boost::get<Point<K>>(&v)) {
				intersections.push_back(*pp);
			}
		}
		return intersections;
	} else if (CGAL::assign(ps, o)) {
		Open_Parabola_segment_2 ops{ps};
		return parabola_intersections(seg, ps.line(), ps.center(), ops.get_p1(), ops.get_p2());
	}
}

std::unordered_set<SDG2::Vertex_handle>
IsolineSimplifier::intersected_region(Segment<K> rung, Point<K> p) {
	bool reversed = m_p_next.contains(rung.target()) && m_p_next.at(rung.target()) == rung.source();
	Point<K> t = reversed ? rung.target() : rung.source();
	Point<K> u = reversed ? rung.source() : rung.target();
	Point<K> s = m_p_prev.at(t);
	Point<K> v = m_p_next.at(u);
	Segment<K> st(s, t);
	Segment<K> tu(t, u);
	Segment<K> uv(u, v);

	Segment<K> sp(s, p);
	Segment<K> pv(p, v);

	std::unordered_set<SDG2::Vertex_handle> region;

	region.insert(m_e_vertex.at(st));
	region.insert(m_e_vertex.at(tu));
	region.insert(m_e_vertex.at(uv));
	region.insert(m_p_vertex.at(s));
	region.insert(m_p_vertex.at(t));
	region.insert(m_p_vertex.at(u));
	region.insert(m_p_vertex.at(v));


	auto add_intersected = [&](Point<K> s, Segment<K> sp) {
		std::stack<SDG2::Vertex_handle> vertex_stack;
		vertex_stack.push(m_p_vertex.at(s));
		std::unordered_set<SDG2::Vertex_handle> visited;

		while (!vertex_stack.empty()) {
			auto current = vertex_stack.top();
			vertex_stack.pop();

			if (visited.contains(current)) {
				continue;
			}
			visited.insert(current);
			region.insert(current);

			auto cit_start = m_delaunay.incident_edges(current);
			auto cit = cit_start;

			do {
				SDG2::Edge e = *cit;
				if (m_delaunay.is_infinite(e)) {
					++cit;
					continue;
				}
				auto inters = intersections_primal(sp, m_delaunay.primal(e));
				if (!inters.empty()) {
					SDG2::Vertex_handle a = e.first->vertex(SDG2::ccw(e.second));
					SDG2::Vertex_handle b = e.first->vertex(SDG2::cw(e.second));
					SDG2::Vertex_handle target = a == current ? b : a;
					vertex_stack.push(target);
				}

				++cit;
			} while (cit != cit_start);
		}
	};

	add_intersected(s, sp);
	add_intersected(v, pv);

	return region;
}

std::pair<std::vector<std::vector<SDG2::Edge>>, int>
IsolineSimplifier::boundaries(const std::unordered_set<SDG2::Vertex_handle>& region) const {
	std::set<SDG2::Edge> edges;
	std::unordered_map<SDG2::Face_handle, std::vector<SDG2::Edge>> f_edge;

	for (const auto& vh : region) {
		auto eit_start = m_delaunay.incident_edges(vh);
		auto eit = eit_start;

		do {
			SDG2::Edge e = *eit;

			SDG2::Vertex_handle a = e.first->vertex(SDG2::ccw(e.second));
			SDG2::Vertex_handle b = e.first->vertex(SDG2::cw(e.second));
			if (region.contains(a) != region.contains(b)) {
				edges.insert(e);
				f_edge[e.first].push_back(e);
				f_edge[e.first->neighbor(e.second)].push_back(e);
			}

			++eit;
		} while (eit != eit_start);
	}

	std::vector<std::vector<SDG2::Edge>> boundaries;
	while (!edges.empty()) {
		std::vector<SDG2::Edge> boundary;
		auto start = *edges.begin();
		auto e = start;

		do {
			boundary.push_back(e);
			edges.erase(e);

			auto& es = f_edge.at(e.first);
			auto next = es[0] == e ? es[1] : es[0];
			e = next;
		} while (e != start);

		boundaries.push_back(boundary);
	}

	if (boundaries.size() == 1) return std::pair(boundaries, 0);
	if (boundaries.size() > 1) {
		auto b1 = boundaries[0];
		auto b2 = boundaries[1];

		int outer = -1;

		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < boundaries.size(); j++) {
				if (i == j) continue;
				SDG2::Edge finite_edge;
				for (auto& e1 : boundaries[i]) {
					if (!m_delaunay.is_infinite(e1)) {
						finite_edge = e1;
					}
				}

				auto p = point_of_Voronoi_edge(finite_edge, m_delaunay);
				int inters_cnt = 0;
				for (auto& e2 : boundaries[j]) {
					if (m_delaunay.is_infinite(e2)) {
						outer = j;
						break;
					}
					auto inters = intersections_primal(Segment<K>(p, p + Vector<K>(100000.0, 100000.0)), m_delaunay.primal(e2));
					inters_cnt += inters.size();
				}
				if (inters_cnt % 2 == 1) {
					outer = j;
					break;
				}
			}
			if (outer >= 0) break;
		}


		if (outer < 0) {
			throw std::runtime_error("Could not determine outer boundary!");
		}
		return std::pair(boundaries, outer);
	}
}

bool IsolineSimplifier::check_ladder_collapse_topology(const SlopeLadder& ladder) {
	std::unordered_set<Point<K>> points_to_skip;

	for (int i = 0; i < ladder.m_rungs.size(); i++) {
		const auto& rung = ladder.m_rungs.at(i);
		points_to_skip.insert(rung.source());
		points_to_skip.insert(rung.target());
	}

	for (int i = 0; i < ladder.m_rungs.size(); i++) {
		if (check_rung_collapse_topology(ladder.m_rungs[i], ladder.m_collapsed[i], points_to_skip)) {
			return true;
		}
	}
	return false;
}

bool IsolineSimplifier::check_rung_collapse_topology(Segment<K> rung, Point<K> p, std::unordered_set<Point<K>>& allowed) {
	auto problem_vertex = [&](SDG2::Vertex_handle vh) {
		if (vh->is_segment()) return false;
		auto x = vh->site().point();
		if (allowed.contains(x)) return false;

		auto reversed = m_p_next.contains(rung.target()) && m_p_next.at(rung.target()) == rung.source();
		auto t = reversed ? rung.target() : rung.source();
		auto u = reversed ? rung.source() : rung.target();
		auto s = m_p_prev.at(t);
		auto v = m_p_next.at(u);

		auto st = Segment<K>(s, t);
		auto tu = Segment<K>(t, u);
		auto uv = Segment<K>(u, v);

		auto sp = Segment<K>(s, p);
		auto pv = Segment<K>(p, v);

		auto closest_spv = CGAL::squared_distance(sp, x) < CGAL::squared_distance(pv, x) ? sp : pv;
		auto spv_o = CGAL::orientation(closest_spv.source(), closest_spv.target(), x);

		auto stuv = std::vector({st, tu, uv});

		Segment<K> closest_stuv = *std::min_element(stuv.begin(), stuv.end(), [&x](const auto& seg1, const auto& seg2){ return CGAL::squared_distance(seg1, x) < CGAL::squared_distance(seg2, x); } );
	    auto stuv_o = CGAL::orientation(closest_stuv.source(), closest_stuv.target(), x);

		if (spv_o != stuv_o)
			return true;
		return false;
	};

	auto region = intersected_region(rung, p);
	auto [boundaries_edges, outer] = boundaries(region);
	if (boundaries_edges.size() <= 1) return false;
	for (int i = 0; i < boundaries_edges.size(); ++i) {
		if (i == outer) continue;

		auto& boundary = boundaries_edges[i];
		auto& e = boundary.front();
		SDG2::Vertex_handle a = e.first->vertex(SDG2::ccw(e.second));
		SDG2::Vertex_handle b = e.first->vertex(SDG2::cw(e.second));
		auto& inner_v = region.contains(b) ? a : b;

		std::stack<SDG2::Vertex_handle> vertex_stack;
		vertex_stack.push(inner_v);

		std::unordered_set<SDG2::Vertex_handle> visited;

		while (!vertex_stack.empty()) {
			auto vh = vertex_stack.top();
			vertex_stack.pop();

			if (visited.contains(vh)) continue;
			visited.insert(vh);

			if (problem_vertex(vh)) {
				return true;
			}

			auto vit_start = m_delaunay.incident_vertices(vh);
			auto vit = vit_start;

			do {
				auto v = vit;
				if (!visited.contains(v) && !region.contains(v)) {
					vertex_stack.push(v);
				}

				++vit;
			} while (vit != vit_start);
		}
	}

	return false;
}

double IsolineSimplifier::total_symmetric_difference() const {
	double total = 0;
	for (int i = 0; i < m_isolines.size(); i++) {
		total += symmetric_difference(m_isolines[i], m_simplified_isolines[i]);
	}
	return total;
}

std::pair<double, double> IsolineSimplifier::average_max_vertex_alignment() const {
	double total = 0.0;
	double max = 0.0;
	int count = 0;

	for (auto& [u, sign_map] : m_matching) {
		for (auto& [sign_u, mi] : sign_map) {
			for (auto& [iso, vs] : mi) {
				for (auto& v : vs) {
					CGAL::Sign sign_v;
					bool found = false;
					for (CGAL::Sign possible_sign_v : {CGAL::LEFT_TURN, CGAL::RIGHT_TURN}) {
						if (m_matching.at(v).contains(possible_sign_v))
							for (const auto& [_, pts] : m_matching.at(v).at(possible_sign_v))
								for (const auto& pt : pts)
									if (pt == u) {
										sign_v = possible_sign_v;
										found = true;
									}
					}
					if (!found) {
						std::cerr << "u: " << u << std::endl;
						std::cerr << "v: " << v << std::endl;
						throw std::runtime_error(
						    "Point u matches to v but not the other way around.");
					}

					double alignment = vertex_alignment(m_p_prev, m_p_next, u, v, sign_u, sign_v);
					max = std::max(max, alignment);
					total += alignment;
					count += 1;
				}
			}
		}
	}

	return { total / count, max };
}

void IsolineSimplifier::clear() {
	m_delaunay.clear();
	m_p_isoline.clear();
	m_p_prev.clear();
	m_p_next.clear();
	m_p_iterator.clear();
	m_p_ladder.clear();
	m_e_ladder.clear();
	m_p_vertex.clear();
	m_e_vertex.clear();
	m_e_intersects.clear();
	m_delaunay.clear();
	m_separator.clear();
	m_matching.clear();
	m_slope_ladders.clear();
}

int IsolineSimplifier::ladder_count() {
	clear();
	initialize_sdg();
	initialize_point_data();
	m_separator = medial_axis_separator(m_delaunay, m_p_isoline, m_p_prev, m_p_next);
	m_matching = matching(m_delaunay, m_separator, m_p_prev, m_p_next, m_p_isoline, m_p_vertex,
	                     m_angle_filter, m_alignment_filter);
	initialize_slope_ladders();
	return m_slope_ladders.size();
}
}
