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

#include <vector>
#include "isoline.h"
#include "medial_axis.h"

#ifndef CARTOCROW_MEDIAL_AXIS_SEPARATOR_H
#define CARTOCROW_MEDIAL_AXIS_SEPARATOR_H

namespace cartocrow::isoline_simplification {
typedef std::unordered_map<Isoline<K>*, std::vector<SDG2::Edge>> Separator;
Separator medial_axis_separator(const SDG2& delaunay, std::vector<Isoline<K>>& isolines);
std::variant<Gt::Point_2, Gt::Segment_2> site_projection(const SDG2& delaunay, const SDG2::Edge& edge, const SDG2::Site_2& site);
Gt::Segment_2 snap_endpoints(Gt::Segment_2 proj, Gt::Segment_2 original);
std::vector<Gt::Segment_2> matching(const SDG2& delaunay, const Separator& edges);
}
#endif //CARTOCROW_MEDIAL_AXIS_SEPARATOR_H
