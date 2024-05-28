// The functions in this file are internal CGAL functions that cannot be accessed so
// have been copied here instead and adapted slightly.
// They originate from:
// https://github.com/CGAL/cgal/blob/96f698ca09b61b6ca7587d43b022a0db43519699/Segment_Delaunay_graph_2/include/CGAL/Segment_Delaunay_graph_2/Segment_Delaunay_graph_2_impl.h#L2320
//
// License of these two functions (incircle, arrangement_type):
//// Copyright (c) 2003,2004,2005,2006  INRIA Sophia-Antipolis (France).
//// All rights reserved.
////
//// This file is part of CGAL (www.cgal.org).
////
//// $URL$
//// $Id$
//// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
////
////
//// Author(s)     : Menelaos Karavelas <mkaravel@iacm.forth.gr>

#include "voronoi_helpers_cgal.h"

namespace cartocrow::isoline_simplification {
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

CGAL::Sign incircle(const SDG2& sdg, const SDG2::Face_handle& f, const SDG2::Site_2& q) {
	if (!sdg.is_infinite(f)) {
		return incircle(sdg, f->vertex(0)->site(), f->vertex(1)->site(), f->vertex(2)->site(), q);
	}

	int inf_i(-1); // to avoid compiler warning
	for (int i = 0; i < 3; i++) {
		if (sdg.is_infinite(f->vertex(i))) {
			inf_i = i;
			break;
		}
	}
	return incircle(sdg, f->vertex(SDG2::ccw(inf_i))->site(), f->vertex(SDG2::cw(inf_i))->site(), q);
}

Gt::Arrangement_type_2::result_type arrangement_type(const SDG2& sdg, const SDG2::Site_2& p,
                                                     const SDG2::Site_2& q) {
	typedef typename Gt::Arrangement_type_2 AT2;
	typedef typename AT2::result_type Arrangement_type;

	Arrangement_type res = sdg.geom_traits().arrangement_type_2_object()(p, q);

	if (res == AT2::TOUCH_INTERIOR_12 || res == AT2::TOUCH_INTERIOR_21 ||
	    res == AT2::TOUCH_INTERIOR_11 || res == AT2::TOUCH_INTERIOR_22) {
		return AT2::DISJOINT;
	}
	if (res == AT2::TOUCH_11 || res == AT2::TOUCH_12 || res == AT2::TOUCH_21 || res == AT2::TOUCH_22) {
		return AT2::DISJOINT;
	}

	return res;
}
}