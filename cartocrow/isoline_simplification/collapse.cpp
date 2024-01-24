//
// Created by steven on 1/23/24.
//

#include "collapse.h"

namespace cartocrow::isoline_simplification {
double signed_area(std::vector<Gt::Point_2> pts) {
	double total = 0.0;
	auto prev = pts.back();
	for (const Gt::Point_2& curr : pts) {
		total += prev.x() * curr.y() - prev.y() * curr.x();
		prev = curr;
	}
	return total;
}

Gt::Line_2 area_preservation_line(Gt::Point_2 s, Gt::Point_2 t, Gt::Point_2 u, Gt::Point_2 v) {
	double area = signed_area({s, v, u, t});
	double base_length = sqrt(squared_distance(s, v));
	double height = 2 * area / base_length;
	Gt::Line_2 base(s, v);
	// TODO: Check LEFT_TURN
	auto vec = base.direction().perpendicular(CGAL::LEFT_TURN).vector();
	vec /= sqrt(vec.squared_length());
	vec *= height;
	CGAL::Aff_transformation_2<K> translate(CGAL::TRANSLATION, vec);
	return base.transform(translate);
}

std::vector<Gt::Point_2> collapse(SlopeLadder& ladder, const PointToPoint& p_prev,
                                  const PointToPoint& p_next) {
	if (ladder.rungs.size() == 1) {
		return {midpoint(ladder.rungs[0])};
	}

	auto a = midpoint(ladder.rungs.front());
	auto b = midpoint(ladder.rungs.back());
	Gt::Line_2 harmony_line(a, b);

	std::vector<Gt::Point_2> new_vertices;

	for (const auto& rung : ladder.rungs) {
		auto t = rung.source();
		auto u = rung.target();
		Gt::Point_2 s;
		if (p_prev.contains(t)) {
			s = p_prev.at(t);
		} else {
			s = t + (t - u);
		}
		Gt::Point_2 v;
		if (p_next.contains(u)) {
			v = p_next.at(u);
		} else {
			v = u + (u - t);
		}

		Gt::Line_2 l = area_preservation_line(s, t, u, v);
		// todo: Handle optional and variant properly...
		Gt::Point_2 new_vertex = boost::get<Gt::Point_2>(*CGAL::intersection(l, harmony_line));
		new_vertices.push_back(new_vertex);
	}

	return new_vertices;
}
}