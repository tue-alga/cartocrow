#ifndef CARTOCROW_POINT_VORONOI_HELPERS_H
#define CARTOCROW_POINT_VORONOI_HELPERS_H

#include "cartocrow/core/core.h"
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Delaunay_triangulation_adaptation_policies_2.h>
#include <CGAL/Delaunay_triangulation_adaptation_traits_2.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Voronoi_diagram_2.h>

using namespace cartocrow;
//typedef CGAL::Delaunay_triangulation_2<K>                                    DT;
//typedef CGAL::Delaunay_triangulation_adaptation_traits_2<DT>                 AT;
//typedef CGAL::Delaunay_triangulation_caching_degeneracy_removal_policy_2<DT> AP;
//typedef CGAL::Voronoi_diagram_2<DT,AT,AP>                                    VD;

template<class K,
         class DT = CGAL::Delaunay_triangulation_2<K>,
         class AT = CGAL::Delaunay_triangulation_adaptation_traits_2<DT>,
         class AP = CGAL::Delaunay_triangulation_caching_degeneracy_removal_policy_2<DT>,
         class VD = CGAL::Voronoi_diagram_2<DT,AT,AP>>
Polygon<K> face_to_polygon(VD vd, typename VD::Face& face, Rectangle<K> bbox) {
	std::vector<Point<K>> pts;
	auto circ_start = face.ccb();
	auto circ = circ_start;

	do {
		auto he = *circ;
		// Cannot easily access the geometry of the halfedge, so go via dual
		const DT& dt = vd.dual();
		auto dte = he.dual();
		auto objE = dt.dual(dte); // line segment, ray, or line
		Segment<K> seg;
		Ray<K> ray;
		Line<K> line;
		Segment<K> segI;
		if (CGAL::assign(seg, objE)) {
			auto objI = CGAL::intersection(seg, bbox);
			segI = CGAL::object_cast<Segment<K>>(objI);
		} else if (CGAL::assign(ray, objE)) {
			auto objI = CGAL::intersection(ray, bbox);
			segI = CGAL::object_cast<Segment<K>>(objI);
		} else if (CGAL::assign(line, objE)) {
			auto objI = CGAL::intersection(line, bbox);
			segI = CGAL::object_cast<Segment<K>>(objI);
		} else {
			throw std::runtime_error("Voronoi edge is neither a segment, ray, nor a line.");
		}
		pts.push_back(segI.source());
	} while (++circ != circ_start);

	return {pts.begin(), pts.end()};
}

#endif //CARTOCROW_POINT_VORONOI_HELPERS_H
