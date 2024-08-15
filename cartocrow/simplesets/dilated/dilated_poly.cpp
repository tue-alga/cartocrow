#include "dilated_poly.h"
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

CSPolygon dilatePattern(const Pattern& pattern, const Number<Inexact>& dilationRadius) {
	auto cont = pattern.contour();

	if (holds_alternative<Polygon<Inexact>>(cont)) {
		auto exactPolygon = makeExact(std::get<Polygon<Inexact>>(cont));

		if (exactPolygon.size() == 1) {
			CSTraits::Rational_circle_2 circle(exactPolygon.vertex(0), dilationRadius * dilationRadius);
			CSTraits traits;
			auto make_x_monotone = traits.make_x_monotone_2_object();
			std::vector<boost::variant<CSTraits::Point_2, CSTraits::X_monotone_curve_2>> curves_and_points;
			make_x_monotone(circle, std::back_inserter(curves_and_points));
			std::vector<CSTraits::X_monotone_curve_2> curves;

			// There should not be any isolated points
			for (auto kinda_curve : curves_and_points) {
				auto curve = boost::get<CSTraits::X_monotone_curve_2>(kinda_curve);
				curves.push_back(curve);
			}

			// todo: is order of curves always correct?
			return {curves.begin(), curves.end()};
		}

		auto dilation = CGAL::approximated_offset_2(exactPolygon, dilationRadius, M_EPSILON);
		if (dilation.has_holes()) {
			throw std::runtime_error("Did not expect holes after dilating a polygonal pattern.");
		}
		return dilation.outer_boundary();
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

		return ccb_to_polygon(*arr.unbounded_face()->inner_ccbs_begin());
	}
}
}
