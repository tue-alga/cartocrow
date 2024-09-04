#include "../types.h"
#include "cartocrow/renderer/render_path.h"

#ifndef CARTOCROW_CS_CURVE_HELPERS_H
#define CARTOCROW_CS_CURVE_HELPERS_H

namespace cartocrow::simplesets {
template <class OutputIterator>
void curveToXMonotoneCurves(const Curve_2& curve, OutputIterator out) {
	CSTraits traits;
	auto make_x_monotone = traits.make_x_monotone_2_object();
	std::vector<boost::variant<CSTraits::Point_2, X_monotone_curve_2>> curves_and_points;
	make_x_monotone(curve, std::back_inserter(curves_and_points));

	// There should not be any isolated points
	for (auto kinda_curve : curves_and_points) {
		if (kinda_curve.which() == 1) {
			*out++ = boost::get<X_monotone_curve_2>(kinda_curve);
		} else {
			std::cout << "Converting curve into x-monotone curves results in isolated point."
			          << std::endl;
			throw std::runtime_error("Cannot convert a degenerate curve into x-monotone curves.");
		}
	}
}

template <class InputIterator, class OutputIterator>
void curvesToXMonotoneCurves(InputIterator begin, InputIterator end, OutputIterator out) {
	for (auto it = begin; it != end; ++it) {
		curveToXMonotoneCurves(*it, out);
	}
}

OneRootPoint nearest(const X_monotone_curve_2& xm_curve, const Point<Exact>& point);

bool liesOn(const Point<Exact>& p, const X_monotone_curve_2& xm_curve);
bool liesOn(const OneRootPoint& p, const X_monotone_curve_2& xm_curve);
renderer::RenderPath renderPathFromXMCurve(const X_monotone_curve_2& xm_curve);
void addCurveToRenderPath(const X_monotone_curve_2& xm_curve, renderer::RenderPath& path, bool& first);
}

#endif //CARTOCROW_CS_CURVE_HELPERS_H
