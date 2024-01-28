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

namespace cartocrow::isoline_simplification {
class IsolineSimplifier {
  public:
	IsolineSimplifier() = default;
	IsolineSimplifier(std::vector<Isoline<K>> isolines);
	std::vector<Isoline<K>> simplify();
	bool step();

	void update_ladders();
	void update_matching();
	void update_separator();

	std::vector<Isoline<K>> m_isolines;
	std::vector<Isoline<K>> m_simplified_isolines;
	PointToIsoline m_p_isoline;
	PointToPoint m_p_prev;
	PointToPoint m_p_next;
	PointToIterator m_p_iterator;
	PointToSlopeLadders m_p_ladder;
	EdgeToSlopeLadder m_e_ladder;
	PointToVertex m_p_vertex;
	EdgeToVertex m_e_vertex;
	SDG2 m_delaunay;
	Separator m_separator;
	Matching m_matching;
	std::vector<std::shared_ptr<SlopeLadder>> m_slope_ladders;
	std::unordered_set<SDG2::Vertex_handle> m_changed_vertices;
	std::vector<Gt::Point_2> m_deleted_points;

  private:
	void initialize_point_data();
	void initialize_sdg();
	void initialize_slope_ladders();
	void collapse_edge(Gt::Segment_2 vertex, Gt::Point_2 new_point);
	void create_slope_ladder(Gt::Segment_2 seg, bool do_push_heap);
};
}

#endif //CARTOCROW_ISOLINE_SIMPLIFICATION_H
