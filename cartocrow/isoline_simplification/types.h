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

#ifndef CARTOCROW_TYPES_H
#define CARTOCROW_TYPES_H

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Segment_Delaunay_graph_2.h>
#include <CGAL/Segment_Delaunay_graph_adaptation_policies_2.h>
#include <CGAL/Segment_Delaunay_graph_adaptation_traits_2.h>
#include <CGAL/Segment_Delaunay_graph_hierarchy_2.h>
#include <CGAL/Segment_Delaunay_graph_traits_2.h>
#include "isoline.h"

namespace cartocrow::isoline_simplification {
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Segment_Delaunay_graph_filtered_traits_without_intersections_2<K, CGAL::Field_with_sqrt_tag> Gt;
typedef CGAL::Segment_Delaunay_graph_hierarchy_2<Gt> SDG2;

typedef std::unordered_map<CGAL::Orientation, std::unordered_map<Isoline<K>*, std::vector<Gt::Point_2>>> MatchedTo;
typedef std::unordered_map<Gt::Point_2, MatchedTo>  Matching;

class SlopeLadder {
  public:
	SlopeLadder() = default;
	std::deque<Gt::Segment_2> rungs;
	std::unordered_map<CGAL::Sign, Gt::Point_2> cap;
};

typedef std::unordered_map<Gt::Point_2, Gt::Point_2*> PointToPoint;
typedef std::unordered_map<Gt::Point_2, Isoline<K>*> PointToIsoline;
typedef std::unordered_map<Gt::Point_2, int> PointToIndex;
typedef std::unordered_map<Gt::Segment_2, SlopeLadder*> EdgeToSlopeLadder;
}
#endif //CARTOCROW_TYPES_H
