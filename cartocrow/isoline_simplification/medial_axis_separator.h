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

#include "isoline.h"
#include "types.h"
#include <vector>

#ifndef CARTOCROW_MEDIAL_AXIS_SEPARATOR_H
#define CARTOCROW_MEDIAL_AXIS_SEPARATOR_H

namespace cartocrow::isoline_simplification {
typedef std::unordered_map<Isoline<K>*, std::vector<SDG2::Edge>> Separator;

std::pair<SDG2::Site_2, SDG2::Site_2> defining_sites(const SDG2::Edge& edge);
SDG2::Point_2 point_of_site(const SDG2::Site_2& site);
Separator medial_axis_separator(const SDG2& delaunay, const PointToIsoline& isoline, const PointToPoint& prev, const PointToPoint& next);
std::variant<Gt::Point_2, Gt::Segment_2> site_projection(const SDG2& delaunay, const SDG2::Edge& edge, const SDG2::Site_2& site);
Gt::Segment_2 snap_endpoints(Gt::Segment_2 proj, Gt::Segment_2 original);
Matching matching(const SDG2& delaunay, const Separator& separator, const PointToPoint& p_prev,
                  const PointToPoint& p_next, const PointToIsoline& p_isoline);
CGAL::Orientation side(const SDG2::Point_2& p, const SDG2::Point_2& point, const PointToPoint& p_prev, const PointToPoint& p_next);
CGAL::Orientation side(const SDG2::Site_2& site, const SDG2::Point_2& point, const PointToPoint& p_prev, const PointToPoint& p_next);
std::vector<Gt::Point_2> project_snap(const SDG2& delaunay, const SDG2::Site_2& site, const SDG2::Edge& edge);
void create_matching(const SDG2& delaunay, const SDG2::Edge& edge, Matching& matching, const PointToPoint& p_prev, const PointToPoint& p_next, const PointToIsoline& p_isoline);
std::function<bool(const Gt::Point_2&, const Gt::Point_2&)> compare_along_isoline(const PointToPoint& p_prev, const PointToPoint& p_next);
}
#endif //CARTOCROW_MEDIAL_AXIS_SEPARATOR_H
