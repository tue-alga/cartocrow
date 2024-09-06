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
void addToRenderPath(const X_monotone_curve_2& xm_curve, renderer::RenderPath& path, bool& first);
Curve_2 toCurve(const X_monotone_curve_2& xmc);

template <class InputIterator>
CSPolycurve arrPolycurveFromXMCurves(InputIterator begin, InputIterator end) {
	PolyCSTraits traits;
	auto construct = traits.construct_curve_2_object();
	std::vector<Curve_2> curves;
	std::transform(begin, end, std::back_inserter(curves), [](const X_monotone_curve_2& xm_curve) {
		return toCurve(xm_curve);
	});
	return construct(curves.begin(), curves.end());
}
}

#endif //CARTOCROW_CS_CURVE_HELPERS_H
