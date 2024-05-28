/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
Copyright (C) 2024 TU Eindhoven

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "symmetric_difference.h"
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/bounding_box.h>

namespace cartocrow::isoline_simplification {

enum Side {
	LEFT = 0,
	BOTTOM = 1,
	RIGHT = 2,
	TOP = 3,
};

Side closest_side(const Point<Exact>& point, const Exact::Iso_rectangle_2& bb) {
	std::vector<double> dist({ to_double(point.x()) - to_double(bb.xmin()), to_double(point.y()) - to_double(bb.ymin()),
	                           to_double(bb.xmax()) - to_double(point.x()), to_double(bb.ymax()) - to_double(point.y()) });
	auto it = std::min_element(dist.begin(), dist.end());
	return static_cast<Side>(std::distance(dist.begin(), it));
}

Vector<Exact> side_direction(const Side& side) {
	switch (side) {
	case LEFT:
		return { -1, 0 };
	case TOP:
		return { 0, 1 };
	case RIGHT:
		return { 1, 0 };
	case BOTTOM:
		return { 0, -1 };
	}
}

Point<Exact> proj_on_side(Point<Exact> p, Side side, const Exact::Iso_rectangle_2& rect) {
	switch (side) {
	case LEFT:
		return { rect.xmin(), p.y() };
	case TOP:
		return { p.x(), rect.ymax() };
	case RIGHT:
		return { rect.xmax(), p.y() };
	case BOTTOM:
		return { p.x(), rect.ymin() };
	}
}

Point<Exact> corner(const Side& side1, const Side& side2, const Exact::Iso_rectangle_2& rect) {
	if (side1 > side2) return corner(side2, side1, rect);
	auto dist = abs(side1 - side2);
	if (dist == 1) {
		return rect.vertex(side1);
	} else {
		return rect.vertex(side2);
	}
}

Side next_side(const Side& side) {
	return static_cast<Side>((side + 1) % 4);
}

Polygon<Exact> close_isoline(const Isoline<K>& isoline, Exact::Iso_rectangle_2& bb, Side source_side, Side target_side) {
	std::vector<Point<Exact>> points;
	std::transform(isoline.m_points.begin(), isoline.m_points.end(), std::back_inserter(points),
				   [](Point<Inexact> p) { return Point<Exact>(p.x(), p.y()); });
	if (!isoline.m_closed) {
		Point<Exact> source = points.front();
		Point<Exact> target = points.back();

		// case distinction on distance between sides
		auto dist = abs(source_side - target_side);
		if (dist > 2) {
			dist -= 2;
		}

		auto source_dir = side_direction(source_side);
		auto target_dir = side_direction(target_side);

		auto source_out = proj_on_side(source, source_side, bb) + source_dir;
		auto target_out = proj_on_side(target, target_side, bb) + target_dir;
		points.push_back(target_out);

		if (dist == 0) {
			// done
		} else if (dist == 1) {
			// add corner
			points.push_back(corner(source_side, target_side, bb) + source_dir + target_dir);
		} else {
			assert(dist == 2);
			// add two corners
			auto between_side = next_side(source_side);
			auto between_dir = side_direction(between_side);
			points.push_back(corner(between_side, target_side, bb) + between_dir + target_dir);
			points.push_back(corner(source_side, between_side, bb) + source_dir + between_dir);
		}
		points.push_back(source_out);
	}
	auto poly = Polygon<Exact>(points.begin(), points.end());
	if (!poly.is_simple()) {
		std::cerr << "Symmetric difference computation: polygon is not simple!" << std::endl;
		for (auto vit = poly.vertices_begin(); vit != poly.vertices_end(); vit++) {
			std::cerr << *vit << std::endl;
		}
	}
	if (poly.is_clockwise_oriented()) {
		poly.reverse_orientation();
	}
	return poly;
}

double symmetric_difference(const Isoline<K>& original, const Isoline<K>& simplified) {
	std::vector<Point<Exact>> all_points;
	std::transform(original.m_points.begin(), original.m_points.end(), std::back_inserter(all_points),
				   [](Point<Inexact> p) { return Point<Exact>(p.x(), p.y()); });
	std::transform(simplified.m_points.begin(), simplified.m_points.end(), std::back_inserter(all_points),
				   [](Point<Inexact> p) { return Point<Exact>(p.x(), p.y()); });

	Exact::Iso_rectangle_2 bb = CGAL::bounding_box(all_points.begin(), all_points.end());

	auto source_side = closest_side(
	    Point<Exact>(original.m_points.front().x(), original.m_points.front().y()), bb);
	auto target_side = closest_side(
	    Point<Exact>(original.m_points.back().x(), original.m_points.back().y()), bb);
	auto p1 = close_isoline(original, bb, source_side, target_side);
	auto p2 = close_isoline(simplified, bb, source_side, target_side);

	std::vector<PolygonWithHoles<Exact>> result_polys;

	symmetric_difference(p1, p2, std::back_inserter(result_polys));

	double total_area = 0.0;
	for (const auto& poly : result_polys) {
		total_area += to_double(abs(poly.outer_boundary().area()));
		if (poly.has_holes()) {
			// polygons should not have holes
			for (const auto& h : poly.holes()) {
				total_area -= to_double(abs(h.area()));
			}
		}
	}

	return total_area;
}

}