#include "triangulation.h"

#include <algorithm>
#include <limits>
#include <CGAL/create_straight_skeleton_from_polygon_with_holes_2.h>
#include <CGAL/Straight_skeleton_2.h>
#include "../core/core.h"

#include <iostream>

namespace cartocrow::mosaic_cartogram {

using StraightSkeleton = CGAL::Straight_skeleton_2<Exact>;

void addOuterTriangle(RegionArrangement &arr) {
	Number<Exact> xMin = std::numeric_limits<double>::max();
	Number<Exact> xMax = std::numeric_limits<double>::min();
	Number<Exact> yMin = std::numeric_limits<double>::max();
	Number<Exact> yMax = std::numeric_limits<double>::min();
	for (auto v : arr.vertex_handles()) {
		auto &p = v->point();
		xMin = std::min(xMin, p.x());
		xMax = std::max(xMax, p.x());
		yMin = std::min(yMin, p.y());
		yMax = std::max(yMax, p.y());
	}

	const auto w = xMax - xMin;
	const auto h = yMax - yMin;

	const Point<Exact> p0(xMin - w,          yMin - h / 2  );  // bottom left
	const Point<Exact> p1(xMax + w,          yMin - h / 2  );  // bottom right
	const Point<Exact> p2((xMin + xMax) / 2, yMax + h * 3/2);  // top

	const auto v0 = arr.insert_in_face_interior(p0, arr.unbounded_face());
	const auto v1 = arr.insert_in_face_interior(p1, arr.unbounded_face());
	const auto v2 = arr.insert_in_face_interior(p2, arr.unbounded_face());

	arr.insert_at_vertices(Segment<Exact>(p0, p1), v0, v1);
	arr.insert_at_vertices(Segment<Exact>(p1, p2), v1, v2);
	arr.insert_at_vertices(Segment<Exact>(p2, p0), v2, v0);
}

Polygon<Exact> ccbToPolygon(const RegionArrangement::Ccb_halfedge_const_circulator &circ) {
	Polygon<Exact> polygon;
	auto curr = circ;
	do {
		polygon.push_back(curr->target()->point());
	} while (++curr != circ);
	return polygon;
}

PolygonWithHoles<Exact> arrangementToPolygon(const RegionArrangement &arr) {
	// create polygon from outer boundary of arrangement
	const auto outerFace = arr.unbounded_face()->holes_begin()->ptr()->twin()->face();
	const auto boundary = ccbToPolygon(outerFace->outer_ccb());  // counterclockwise!
	PolygonWithHoles<Exact>	polygon(boundary);

	// add all holes to polygon
	for (auto hit = outerFace->holes_begin(); hit != outerFace->holes_end(); ++hit)
		polygon.add_hole(ccbToPolygon(*hit));  // clockwise!

	return polygon;
}

RegionArrangement triangulate(const RegionArrangement &arrOrig) {
	RegionArrangement arr(arrOrig);
	addOuterTriangle(arr);

	const auto polygon = arrangementToPolygon(arr);
	const StraightSkeleton skeleton = *CGAL::create_interior_straight_skeleton_2(polygon, Exact());

	for (auto eit = skeleton.halfedges_begin(); eit != skeleton.halfedges_end(); ++eit) {
		if (!eit->is_border()) {
			const auto p = eit->vertex()->point();
			const auto q = eit->opposite()->vertex()->point();
			CGAL::insert(arr, Segment<Exact>(p, q));
		}
	}

	return arr;
}

} // namespace cartocrow::mosaic_cartogram
