#include "triangulation.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <CGAL/create_straight_skeleton_from_polygon_with_holes_2.h>
#include <CGAL/Straight_skeleton_2.h>

#include "../core/core.h"
#include "graph.h"

namespace cartocrow::mosaic_cartogram {

using StraightSkeleton = CGAL::Straight_skeleton_2<Exact>;

std::vector<Point<Exact>> addOuterTriangle(RegionArrangement &arr) {
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

	// defined such that there is one large sea region "on top", which will become the root of the OST
	const Point<Exact> p0(xMin - w,          yMax + h / 2  );  // top left
	const Point<Exact> p1((xMin + xMax) / 2, yMin - h * 3/2);  // bottom
	const Point<Exact> p2(xMax + w,          yMax + h / 2  );  // top right

	const auto v0 = arr.insert_in_face_interior(p0, arr.unbounded_face());
	const auto v1 = arr.insert_in_face_interior(p1, arr.unbounded_face());
	const auto v2 = arr.insert_in_face_interior(p2, arr.unbounded_face());

	arr.insert_at_vertices(Segment<Exact>(p0, p1), v0, v1);
	arr.insert_at_vertices(Segment<Exact>(p1, p2), v1, v2);
	arr.insert_at_vertices(Segment<Exact>(p2, p0), v2, v0);

	return { p0, p1, p2 };
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

int labelSeaRegions(RegionArrangement &arr) {
	auto circ = arr.unbounded_face()->holes_begin()->ptr()->ccb();

	// find topmost edge (such that "_outer0" will be the topmost region)
	while (circ->source()->point().y() != circ->target()->point().y()) ++circ;

	// label outer sea regions in counterclockwise order
	int i = 0;
	auto curr = circ;
	do {
		curr->twin()->face()->set_data("_outer" + std::to_string(i++));
	} while (--curr != circ);

	// label remaining sea regions
	i = 0;
	for (const auto f : arr.face_handles())
		if (!f->is_unbounded() && f->data().empty())
			f->set_data("_sea" + std::to_string(i++));

	return i;
}

int triangulate(RegionArrangement &arr, const std::vector<Point<Exact>> &salientPoints) {
	const auto outerPoints = addOuterTriangle(arr);
	const auto polygon = arrangementToPolygon(arr);
	const StraightSkeleton skeleton = *CGAL::create_interior_straight_skeleton_2(polygon, Exact());

	// anchors = salientPoints \union outerPoints
	std::vector<Point<Exact>> anchors(salientPoints);
	anchors.insert(anchors.end(), outerPoints.begin(), outerPoints.end());

	std::vector<Point<Exact>> indexToPoint;
	for (auto v : skeleton.vertex_handles())
		if (v->is_skeleton())
			indexToPoint.push_back(v->point());

	const int numberOfSkeletonPoints = indexToPoint.size();

	// after this, `indexToPoint` first contains all skeleton points, then all anchor points
	indexToPoint.insert(indexToPoint.end(), anchors.begin(), anchors.end());

	// graph size = no. skeleton vertices + no. anchor vertices
	UndirectedGraph graph(indexToPoint.size());

	// add all inner bisectors and all anchor-incident contour bisectors to the graph
	int numberOfBisectorsAdded = 0;
	for (auto eit = skeleton.halfedges_begin(); eit != skeleton.halfedges_end(); ++eit) {
		const auto p1 = eit->vertex()->point();
		const auto p2 = eit->opposite()->vertex()->point();

		// (temp) bad, slow
		int i1 = 0;
		int i2 = 0;
		while (i1 < graph.getNumberOfVertices() && indexToPoint[i1] != p1) i1++;
		while (i2 < graph.getNumberOfVertices() && indexToPoint[i2] != p2) i2++;

		// immediately skip edges incident to a vertex which is not an anchor or a skeleton vertex
		if (std::max(i1, i2) >= graph.getNumberOfVertices()) continue;

		// only process one of each pair of halfedges
		if (i1 > i2) continue;

		if (eit->is_inner_bisector()) {
			graph.addEdge(i1, i2);
		} else if (eit->is_bisector()) {
			// so if eit is contour bisector:
			if (std::max(i1, i2) >= numberOfSkeletonPoints) {
				// so if either p1 or p2 is an anchor:
				graph.addEdge(i1, i2);
				numberOfBisectorsAdded++;
			}
		}
	}

	// (temp) debug message
	std::cerr << "[info] added " << numberOfBisectorsAdded << " bisectors for " << salientPoints.size() << " salient points (and 3 outer points)" << std::endl;

	// remove all non-anchor deg=1 vertices until none are left
	// TODO: use queue of candidates to improve runtime
	for (int i = 0; i < numberOfSkeletonPoints; i++) {
		if (graph.getDegree(i) == 1) {
			graph.isolate(i);
			i = -1;  // there may be new candidates; restart
		}
	}

	// add all remaining edges to arrangement
	for (auto [i1, i2] : graph.getEdges()) {
		const auto p1 = indexToPoint[i1];
		const auto p2 = indexToPoint[i2];
		CGAL::insert(arr, Segment<Exact>(p1, p2));
	}

	return labelSeaRegions(arr);
}

bool dualIsTriangular(const RegionArrangement &arr) {
	return false;
}

} // namespace cartocrow::mosaic_cartogram
