#include "types.h"

namespace cartocrow::simplesets {
std::vector<Point<Exact>> makeExact(const std::vector<Point<Inexact>>& points) {
	std::vector<Point<Exact>> exact_points;
	std::transform(points.begin(), points.end(), std::back_inserter(exact_points),
	               [](const Point<Inexact>& pt) { return makeExact(pt); });
	return exact_points;
}

Point<Exact> makeExact(const Point<Inexact>& point) {
	return {point.x(), point.y()};
}

Polygon<Exact> makeExact(const Polygon<Inexact>& polygon) {
	std::vector<Point<Exact>> exact_points;
	std::transform(polygon.vertices_begin(), polygon.vertices_end(), std::back_inserter(exact_points),
	               [](const Point<Inexact>& pt) { return makeExact(pt); });
	return {exact_points.begin(), exact_points.end()};
}

Point<Inexact> approximateAlgebraic(const CSTraits::Point_2& algebraic_point) {
	return {CGAL::to_double(algebraic_point.x()), CGAL::to_double(algebraic_point.y())};
}

CSPolygon circleToCSPolygon(const Circle<Exact>& circle) {
	CSTraits traits;
	auto make_x_monotone = traits.make_x_monotone_2_object();
	std::vector<boost::variant<CSTraits::Point_2, X_monotone_curve_2>> curves_and_points;
	make_x_monotone(circle, std::back_inserter(curves_and_points));
	std::vector<X_monotone_curve_2> curves;

	// There should not be any isolated points
	for (auto kinda_curve : curves_and_points) {
		if (kinda_curve.which() == 1) {
			auto curve = boost::get<X_monotone_curve_2>(kinda_curve);
			curves.push_back(curve);
		} else {
			std::cout << "Splitting circle into x-monotone curves results in isolated point." << std::endl;
			std::cout << circle.center() << "  squared radius: " << circle.squared_radius() << std::endl;
			throw std::runtime_error("Cannot convert circle of radius 0 into a polygon");
		}
	}

	// todo: is order of curves always correct? Because of weird error with intersection delay.
	return CSPolygon{curves.begin(), curves.end()};
}
}
