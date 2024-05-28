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

#include "isoline.h"
#include "types.h"
#include <vector>
#include "voronoi_helpers_cgal.h"

#ifndef CARTOCROW_MEDIAL_AXIS_SEPARATOR_H
#define CARTOCROW_MEDIAL_AXIS_SEPARATOR_H

namespace cartocrow::isoline_simplification {
typedef std::unordered_map<Isoline<K>*, std::vector<SDG2::Edge>> Separator;

class Open_Parabola_segment_2 : public CGAL::Parabola_segment_2<Gt> {
  public:
	Point<K> get_p1() {
		return p1;
	}
	Point<K> get_p2() {
		return p2;
	}
};

std::pair<SDG2::Site_2, SDG2::Site_2> defining_sites(const SDG2::Edge& edge);
SDG2::Point_2 point_of_site(const SDG2::Site_2& site);
Separator medial_axis_separator(const SDG2& delaunay, const PointToIsoline& isoline, const PointToPoint& prev, const PointToPoint& next);
std::variant<Point<K>, Segment<K>> site_projection(const SDG2& delaunay, const SDG2::Edge& edge, const SDG2::Site_2& site);
Segment<K> snap_endpoints(Segment<K> proj, Segment<K> original);
Matching matching(const SDG2& delaunay, const Separator& separator, const PointToPoint& p_prev,
                  const PointToPoint& p_next, const PointToIsoline& p_isoline, const PointToVertex& p_vertex,
                  const double angle_filter, const double alignment_filter);
CGAL::Orientation side(const SDG2::Point_2& p, const SDG2::Point_2& point, const PointToPoint& p_prev, const PointToPoint& p_next);
CGAL::Orientation side(const SDG2::Site_2& site, const SDG2::Point_2& point, const PointToPoint& p_prev, const PointToPoint& p_next);
std::vector<Point<K>> project_snap(const SDG2& delaunay, const SDG2::Site_2& site, const SDG2::Edge& edge);
void create_matching(const SDG2& delaunay, const SDG2::Edge& edge, Matching& matching, const PointToPoint& p_prev,
                     const PointToPoint& p_next, const PointToIsoline& p_isoline, const PointToVertex& p_vertex, const double angle_filter, const double alignment_filter);
std::function<bool(const Point<K>&, const Point<K>&)> compare_along_isoline(const PointToPoint& p_prev, const PointToPoint& p_next);
Point<K> point_of_Voronoi_edge(const SDG2::Edge& edge, const SDG2& delaunay);
std::optional<Segment<K>> check_segment_intersections_Voronoi(const SDG2& delaunay, const Segment<K> seg,
																 const SDG2::Vertex_handle endpoint_handle,
																 const std::unordered_set<SDG2::Vertex_handle>& allowed,
																 const std::optional<SDG2::Vertex_handle> collinear_vertex);
K::Vector_2 normal(const SDG2::Point_2& p, const PointToPoint& p_prev, const PointToPoint& p_next, CGAL::Sign side);
double vertex_alignment(const PointToPoint& p_prev, const PointToPoint& p_next, Point<K> u, Point<K> v, CGAL::Sign uv_side, CGAL::Sign vu_side);
}
#endif //CARTOCROW_MEDIAL_AXIS_SEPARATOR_H
