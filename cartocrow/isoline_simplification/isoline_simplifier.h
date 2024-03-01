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

#ifndef CARTOCROW_ISOLINE_SIMPLIFICATION_H
#define CARTOCROW_ISOLINE_SIMPLIFICATION_H
#include "isoline.h"
#include "medial_axis_separator.h"
#include "types.h"
#include "collapse.h"
#include <boost/heap/d_ary_heap.hpp>

namespace cartocrow::isoline_simplification {
struct slope_ladder_comp {
	inline bool operator()(const std::shared_ptr<SlopeLadder>& sl1, const std::shared_ptr<SlopeLadder>& sl2) const {
		return sl1->m_cost > sl2->m_cost;
	}
};

typedef std::optional<std::variant<Gt::Segment_2, std::monostate>> IntersectionResult;

typedef boost::heap::d_ary_heap<std::shared_ptr<SlopeLadder>, boost::heap::arity<2>, boost::heap::compare<slope_ladder_comp>, boost::heap::mutable_<true>> Heap;
typedef std::unordered_map<std::shared_ptr<SlopeLadder>, Heap::handle_type> LadderToHandle;

class IsolineSimplifier {
  public:
	IsolineSimplifier(std::vector<Isoline<K>> isolines, double angle_filter = M_PI/6, double alignment_filter = 100.0,
	                  std::shared_ptr<LadderCollapse> collapse = std::make_shared<SplineCollapse>(3));
	bool simplify(int target, bool debug = false);
	bool dyken_simplify(int target, double sep_dist = 1);
	bool step();
	std::optional<std::shared_ptr<SlopeLadder>> get_next_ladder();

	void update_ladders();
	void update_matching();

	std::vector<Isoline<K>> m_isolines;
	std::vector<Isoline<K>> m_simplified_isolines;
	PointToIsoline m_p_isoline;
	PointToPoint m_p_prev;
	PointToPoint m_p_next;
	PointToIterator m_p_iterator;
	PointToSlopeLadders m_p_ladder; // maps point to the ladders it is a cap of
	EdgeToSlopeLadders m_e_ladder;
	PointToVertex m_p_vertex;
	EdgeToVertex m_e_vertex;
	EdgeToSlopeLadders m_e_intersects;
	SDG2 m_delaunay;
	Separator m_separator;
	Matching m_matching;
//	std::vector<std::shared_ptr<SlopeLadder>> m_slope_ladders;
	Heap m_slope_ladders;
	LadderToHandle m_ladder_heap_handle;
	std::unordered_set<SDG2::Vertex_handle> m_changed_vertices;
	std::vector<Gt::Point_2> m_deleted_points;
	int m_current_complexity = 0;
	bool m_started = false;
	double m_angle_filter;
	double m_alignment_filter;
	std::shared_ptr<LadderCollapse> m_collapse_ladder;
	bool check_ladder_intersections_naive(const SlopeLadder& ladder) const;
	IntersectionResult check_ladder_intersections_Voronoi(const SlopeLadder& ladder);
	std::unordered_set<SDG2::Vertex_handle> intersected_region(Gt::Segment_2 rung, Gt::Point_2 p);
	std::pair<std::vector<std::vector<SDG2::Edge>>, int>
	boundaries(const std::unordered_set<SDG2::Vertex_handle>& region) const;
	bool check_rung_collapse_topology(Gt::Segment_2 rung, Gt::Point_2 p, std::unordered_set<Gt::Point_2>& allowed);
	bool check_ladder_collapse_topology(const SlopeLadder& ladder);
	double total_symmetric_difference() const;
	std::pair<double, double> average_max_vertex_alignment() const;
	int ladder_count();
	void clear();

  private:
	void initialize_point_data();
	void initialize_sdg();
	void initialize_slope_ladders();
	void collapse_ladder(SlopeLadder& ladder);
	void create_slope_ladder(Gt::Segment_2 seg);
	void check_valid();
	void clean_isolines();
	void remove_ladder_e(Gt::Segment_2 seg);
	std::optional<std::shared_ptr<SlopeLadder>> next_ladder();
//	std::optional<std::shared_ptr<SlopeLadder>> naive_next_ladder();

};
}

#endif //CARTOCROW_ISOLINE_SIMPLIFICATION_H
