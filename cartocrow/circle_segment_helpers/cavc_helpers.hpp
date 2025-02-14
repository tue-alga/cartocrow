#ifndef CARTOCROW_CAVC_HELPERS_HPP
#define CARTOCROW_CAVC_HELPERS_HPP

#include "cs_types.h"
#include <cavc/polylineoffset.hpp>

namespace cartocrow {
// X monotone curves as input
template <class InputIterator>
cavc::Polyline<double> cavcPolyline(InputIterator start, InputIterator end, bool closed) {
	cavc::Polyline<double> polyline;
	auto processCurve = [&polyline](const CSXMCurve& xmCurve) {
		Point<Inexact> s = approximateOneRootPoint(xmCurve.source());
		Point<Inexact> t = approximateOneRootPoint(xmCurve.target());
		if (xmCurve.is_linear()) {
			polyline.addVertex(s.x(), s.y(), 0);
		} else {
			auto circle = approximate(xmCurve.supporting_circle());
			auto center = circle.center();
			auto mid = CGAL::midpoint(s, t);
			auto d = sqrt(CGAL::squared_distance(mid, center));
			Number<Inexact> r = sqrt(circle.squared_radius());
			auto bulge = (r - d) / sqrt(CGAL::squared_distance(mid, s));
			if (bulge > 1) {
				bulge = 1;
			}
			if (bulge < -1) {
				bulge = -1;
			}
			auto orientation = xmCurve.orientation();
			auto sign = orientation == CGAL::COUNTERCLOCKWISE ? 1 : -1;
			polyline.addVertex(s.x(), s.y(), sign * bulge);
		}
	};

	for (auto cit = start; cit != end; ++cit) {
		processCurve(*cit);
	}

	if (closed) {
		polyline.isClosed() = true;
	} else {
		auto last = end;
		--last;
		Point<Inexact> t = approximateOneRootPoint(last->target());
		polyline.addVertex(t.x(), t.y(), 0);
	}

	return polyline;
}
}

#endif //CARTOCROW_CAVC_HELPERS_HPP
