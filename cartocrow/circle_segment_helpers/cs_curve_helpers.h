#include "cs_types.h"

#ifndef CARTOCROW_CS_CURVE_HELPERS_H
#define CARTOCROW_CS_CURVE_HELPERS_H

namespace cartocrow {
/// Return the the point on the circle closest to the provided point.
/// Note that this point may have irrational coordinates.
OneRootPoint closestOnCircle(const Circle<Exact>& circle, const Point<Exact>& point);

/// Convert a CSCurve to CSXMCurves.
template <class OutputIterator>
void curveToXMonotoneCurves(const CSCurve& curve, OutputIterator out) {
	ArrCSTraits traits;
	auto make_x_monotone = traits.make_x_monotone_2_object();
	std::vector<boost::variant<ArrCSTraits::Point_2, CSXMCurve>> curves_and_points;
	make_x_monotone(curve, std::back_inserter(curves_and_points));

	// There should not be any isolated points
	for (auto kinda_curve : curves_and_points) {
		if (kinda_curve.which() == 1) {
			*out++ = boost::get<CSXMCurve>(kinda_curve);
		} else {
			std::cout << "Converting curve into x-monotone curves results in isolated point."
			          << std::endl;
			throw std::runtime_error("Cannot convert a degenerate curve into x-monotone curves.");
		}
	}
}

/// Convert a range of CSCurves to CSXMCurves.
template <class InputIterator, class OutputIterator>
void curvesToXMonotoneCurves(InputIterator begin, InputIterator end, OutputIterator out) {
	for (auto it = begin; it != end; ++it) {
		curveToXMonotoneCurves(*it, out);
	}
}

/// Return the point on the CSXMCurve nearest to the provided point.
OneRootPoint nearest(const CSXMCurve& xm_curve, const Point<Exact>& point);

/// Check whether the point lies on the CSXMCurve.
/// Note that this is simply a wrapper around the point_position function of CSXMCurve.
bool liesOn(const Point<Exact>& p, const CSXMCurve& xm_curve);
/// Check whether the point lies on the CSXMCurve.
/// Note that this is simply a wrapper around the point_position function of CSXMCurve.
bool liesOn(const OneRootPoint& p, const CSXMCurve& xm_curve);
/// Convert a CSXMCurve to a CSCurve.
CSCurve toCurve(const CSXMCurve& xmc);

/// Convert a range of CSCurves to CSXMCurves.
/// This function merges consecutive CSXMCurves that lie on the same supporting geometry (line or circle).
template <class OutputIterator, class InputIterator>
void toCurves(InputIterator begin, InputIterator end, OutputIterator out) {
	std::optional<CSCurve> lastCurve;
	for (auto curr = begin; curr != end; ++curr) {
		CSXMCurve xmc = *curr;
		if (!lastCurve.has_value()) {
			lastCurve = toCurve(xmc);
		} else {
			if (lastCurve->is_linear() && xmc.is_linear() && lastCurve->supporting_line() == xmc.supporting_line()) {
				CSCurve newCurve(lastCurve->supporting_line(), lastCurve->source(), xmc.target());
				lastCurve = newCurve;
			} else if (lastCurve->is_circular() && xmc.is_circular() && lastCurve->supporting_circle() == xmc.supporting_circle()) {
				CSCurve newCurve;
				if (xmc.target() == lastCurve->source()) {
					newCurve = CSCurve(lastCurve->supporting_circle());
				} else {
					newCurve = CSCurve(lastCurve->supporting_circle(), lastCurve->source(), xmc.target());
				}
				lastCurve = newCurve;
			} else {
				++out = *lastCurve;
				lastCurve = toCurve(xmc);
			}
		}
	}
	if (lastCurve.has_value()) {
		++out = *lastCurve;
	}
}

/// Convert a range of CSXMCurves to a CSPolycurve.
template <class InputIterator>
CSPolycurve arrPolycurveFromXMCurves(InputIterator begin, InputIterator end) {
	PolycurveCSTraits traits;
	auto construct = traits.construct_curve_2_object();
	std::vector<CSCurve> curves;
	std::transform(begin, end, std::back_inserter(curves), [](const CSXMCurve& xm_curve) {
		return toCurve(xm_curve);
	});
	return construct(curves.begin(), curves.end());
}

/// Return true iff c1 overlaps c2 and c1 is not a superset of c2.
bool liesOn(const CSXMCurve& c1, const CSXMCurve& c2);

/// Return an approximate tangent at the start of the curve.
Vector<Inexact> startTangent(const CSXMCurve& c);
/// Return an approximate tangent at the end of the curve.
Vector<Inexact> endTangent(const CSXMCurve& c);

/// Return the approximate turning angle along the curve.
double approximateTurningAngle(const CSXMCurve& xmc);

/// Return the approximate length of the curve.
Number<Inexact> approximateLength(const CSXMCurve& xmc);

/// Return the approximate length of the curve.
Number<Inexact> approximateLength(const CSCurve& c);
}

#endif //CARTOCROW_CS_CURVE_HELPERS_H
