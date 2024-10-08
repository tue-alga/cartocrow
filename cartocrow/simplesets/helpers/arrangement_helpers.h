#ifndef CARTOCROW_ARRANGEMENT_HELPERS_H
#define CARTOCROW_ARRANGEMENT_HELPERS_H

#include <vector>
#include <CGAL/General_polygon_2.h>

template <class Traits, class Ccb>
CGAL::General_polygon_2<Traits> ccb_to_polygon(Ccb ccb) {
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
	} while(++curr != ccb);

	return {x_monotone_curves.begin(), x_monotone_curves.end()};
}

#endif //CARTOCROW_ARRANGEMENT_HELPERS_H
