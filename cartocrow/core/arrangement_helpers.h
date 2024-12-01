#ifndef CARTOCROW_ARRANGEMENT_HELPERS_H
#define CARTOCROW_ARRANGEMENT_HELPERS_H

#include "core.h"
#include <vector>
#include <CGAL/General_polygon_2.h>

namespace cartocrow {
template <class Traits, class Ccb>
CGAL::General_polygon_2<Traits> ccb_to_general_polygon(Ccb ccb) {
	Traits traits;
	auto opposite = traits.construct_opposite_2_object();
	auto curr = ccb;

	std::vector<typename Traits::X_monotone_curve_2> x_monotone_curves;
	do {
		auto curve = curr->curve();
		if (curr->source()->point() == curve.source()) {
			x_monotone_curves.push_back(curve);
		} else {
			x_monotone_curves.push_back(opposite(curve));
		}
	} while (++curr != ccb);

	return {x_monotone_curves.begin(), x_monotone_curves.end()};
}

template <class K, class Ccb>
Polygon<K> ccb_to_polygon(Ccb ccb) {
	auto curr = ccb;

	std::vector<Point<K>> points;
	do {
		points.push_back(curr->source()->point());
	} while (++curr != ccb);

	return {points.begin(), points.end()};
}

template <class K, class FaceH>
PolygonWithHoles<K> face_to_polygon_with_holes(FaceH fh) {
	Polygon<K> outer;
	if (fh->has_outer_ccb()) {
		outer = ccb_to_polygon<K>(fh->outer_ccb());
	}
	std::vector<Polygon<K>> holes;
	for (auto ccbIt = fh->inner_ccbs_begin(); ccbIt != fh->inner_ccbs_end(); ++ccbIt) {
		holes.push_back(ccb_to_polygon<K>(*ccbIt));
	}

	return {outer, holes.begin(), holes.end()};
}
}

#endif //CARTOCROW_ARRANGEMENT_HELPERS_H
