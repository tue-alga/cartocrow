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

#ifndef CARTOCROW_MEDIAL_AXIS_HELPERS_H
#define CARTOCROW_MEDIAL_AXIS_HELPERS_H

#include <CGAL/Segment_Delaunay_graph_2.h>
#include <CGAL/Segment_Delaunay_graph_adaptation_policies_2.h>
#include <CGAL/Segment_Delaunay_graph_adaptation_traits_2.h>
#include <CGAL/Segment_Delaunay_graph_hierarchy_2.h>
#include <CGAL/Segment_Delaunay_graph_traits_2.h>

template<class K,
          class Gt  = CGAL::Segment_Delaunay_graph_filtered_traits_without_intersections_2<K, CGAL::Field_with_sqrt_tag>,
          class SDG = CGAL::Segment_Delaunay_graph_hierarchy_2<Gt>,
          class AT  = CGAL::Segment_Delaunay_graph_adaptation_traits_2<SDG>>
bool same_points(const SDG& dg, const typename AT::Site_2& p, const typename AT::Site_2& q) {
	return dg.geom_traits().equal_2_object()(p, q);
}

template<class K,
          class Gt  = CGAL::Segment_Delaunay_graph_filtered_traits_without_intersections_2<K, CGAL::Field_with_sqrt_tag>,
          class SDG = CGAL::Segment_Delaunay_graph_hierarchy_2<Gt>,
          class AT  = CGAL::Segment_Delaunay_graph_adaptation_traits_2<SDG>>
bool is_endpoint_of_segment(const SDG& dg, typename AT::Site_2& p, typename AT::Site_2& s) {
	CGAL_precondition( p.is_point() && s.is_segment() );
	return ( same_points<K>(dg, p, s.source_site()) ||
	        same_points<K>(dg, p, s.target_site()) );
}

template <class Stream,
          class K,
          class Gt  = CGAL::Segment_Delaunay_graph_filtered_traits_without_intersections_2<K, CGAL::Field_with_sqrt_tag>,
          class SDG = CGAL::Segment_Delaunay_graph_hierarchy_2<Gt>>
Stream& draw_dual_edge(const SDG& dg, typename SDG::Edge e, Stream& str)
{
	typename Gt::Line_2    l;
	typename Gt::Segment_2 s;
	typename Gt::Ray_2     r;
	CGAL::Parabola_segment_2<Gt> ps;

	CGAL::Object o = dg.primal(e);

	if (CGAL::assign(l, o))   str << l;
	if (CGAL::assign(s, o))   str << s;
	if (CGAL::assign(r, o))   str << r;
	if (CGAL::assign(ps, o))  str << ps;

	return str;
}

template <class Stream,
          class K,
          class Gt  = CGAL::Segment_Delaunay_graph_filtered_traits_without_intersections_2<K, CGAL::Field_with_sqrt_tag>,
          class SDG = CGAL::Segment_Delaunay_graph_hierarchy_2<Gt>>
Stream& draw_skeleton(const SDG& dg, Stream& str) {
	auto eit = dg.finite_edges_begin();
	for (; eit != dg.finite_edges_end(); ++eit) {
		auto p = eit->first->vertex(  SDG::cw(eit->second) )->site();
		auto q = eit->first->vertex( SDG::ccw(eit->second) )->site();

		bool is_endpoint_of_seg =
		    ( p.is_segment() && q.is_point() &&
		     is_endpoint_of_segment<K>(dg, q, p) ) ||
		    ( p.is_point() && q.is_segment() &&
		     is_endpoint_of_segment<K>(dg, p, q) );

		if ( !is_endpoint_of_seg ) {
			draw_dual_edge<Stream, K>(dg, *eit, str);
		}
	}
	return str;
}

#endif //CARTOCROW_MEDIAL_AXIS_HELPERS_H
