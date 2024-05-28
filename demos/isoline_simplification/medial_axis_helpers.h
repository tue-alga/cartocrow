// These functions are adapted from CGAL functions that fall under the following license.
// Copyright (c) 2003,2004,2005,2006  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/v5.4/Segment_Delaunay_graph_2/include/CGAL/Segment_Delaunay_graph_2.h $
// $Id: Segment_Delaunay_graph_2.h 98e4718 2021-08-26T11:33:39+02:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Menelaos Karavelas <mkaravel@iacm.forth.gr>

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
	Line<K>    l;
	Segment<K> s;
	Ray<K>     r;
	CGAL::Parabola_segment_2<Gt> ps;

	if (dg.is_infinite(e)) return str;
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

template <class Stream,
          class K,
          class Gt  = CGAL::Segment_Delaunay_graph_filtered_traits_without_intersections_2<K, CGAL::Field_with_sqrt_tag>,
          class SDG = CGAL::Segment_Delaunay_graph_hierarchy_2<Gt>>
Stream& draw_dual(const SDG& dg, Stream& str) {
	auto eit = dg.finite_edges_begin();
	for (; eit != dg.finite_edges_end(); ++eit) {
		draw_dual_edge<Stream, K>(dg, *eit, str);
	}
	return str;
}

#endif //CARTOCROW_MEDIAL_AXIS_HELPERS_H
