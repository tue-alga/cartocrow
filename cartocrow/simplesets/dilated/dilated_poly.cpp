#include "dilated_poly.h"
#include "../helpers/cs_polygon_helpers.h"
#include <CGAL/approximated_offset_2.h>

namespace cartocrow::simplesets {
CSPolygon dilateSegment(const Segment<Inexact>& segment, const Number<Inexact>& dilationRadius) {
	std::vector<Point<Exact>> points({makeExact(segment.source()), makeExact(segment.target())});
	Polygon<Exact> polygon(points.begin(), points.end());
	auto dilation = CGAL::approximated_offset_2(polygon, dilationRadius, M_EPSILON);
	return dilation.outer_boundary();
}

CSPolygon ccb_to_polygon(CSArrangement::Ccb_halfedge_const_circulator circ) {
	std::vector<CSArrangement::X_monotone_curve_2> curves;
	auto curr = circ;
	do {
		curves.push_back(curr->curve());
	} while (++curr != circ);
	CSPolygon poly(curves.begin(), curves.end());
	return poly;
}

Dilated::Dilated(const PolyPattern& polyPattern, const Number<Inexact>& dilationRadius) {
	m_catPoints = polyPattern.catPoints();

	auto cont = polyPattern.poly();

	if (holds_alternative<Polygon<Inexact>>(cont)) {
		auto exactPolygon = makeExact(std::get<Polygon<Inexact>>(cont));

		if (exactPolygon.size() == 1) {
			CSTraits::Rational_circle_2 circle(exactPolygon.vertex(0), dilationRadius * dilationRadius);
			m_contour = circleToCSPolygon(circle);
			return;
		}

		auto dilation = CGAL::approximated_offset_2(exactPolygon, dilationRadius, M_EPSILON);
		if (dilation.has_holes()) {
			throw std::runtime_error("Did not expect holes after dilating a polygonal pattern.");
		}
		m_contour = dilation.outer_boundary();
	} else if (holds_alternative<Polyline<Inexact>>(cont)) {
		// 1. Dilate each segment
		// 2. Make arrangement of dilated segments
		// 3. Traverse and extract outer boundary
		// (Assume that the dilation result has no holes.
		// To ensure this we need to constrain the (relative) point size.)

		CSArrangement arr;

		auto polyline = std::get<Polyline<Inexact>>(cont);
		for (auto eit = polyline.edges_begin(); eit != polyline.edges_end(); ++eit) {
			auto seg = *eit;
			auto dilated = dilateSegment(seg, dilationRadius);
			for (auto cit = dilated.curves_begin(); cit != dilated.curves_end(); ++cit) {
				CGAL::insert(arr, *cit);
			}
		}

		m_contour = ccb_to_polygon(*arr.unbounded_face()->inner_ccbs_begin());
	} else {
		throw std::runtime_error("Unknown pattern poly.");
	}
}

const std::vector<CatPoint>& Dilated::catPoints() const {
	return m_catPoints;
}

std::variant<Polyline<Inexact>, Polygon<Inexact>, CSPolygon> Dilated::contour() const {
	return m_contour;
}
}
