/*
Copyright (C) 2026  TU Eindhoven

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

#pragma once

#include <cartocrow/core/core.h>
#include <cartocrow/core/transform_helpers.h>

#include <CGAL/Multipolygon_with_holes_2.h>

namespace cartocrow {
template <class K, class Container_ = std::vector<typename K::Point_2>>
struct MultipolygonWithHoles : public CGAL::Multipolygon_with_holes_2<K, Container_> {
	MultipolygonWithHoles<K, Container_> transform(const CGAL::Aff_transformation_2<K>& trans) const {
		MultipolygonWithHoles<K, Container_> transformed;
		for (const auto& pgn : this->polygons_with_holes()) {
			transformed.add_polygon_with_holes(cartocrow::transform(trans, pgn));
		}
		return transformed;
	}

	Box bbox() const {
		return CGAL::bbox_2(this->polygons_with_holes_begin(), this->polygons_with_holes_end());
	}

	PolygonSet<K, Container_> polygonSet() const {
		PolygonSet<K, Container_> polygonSet;
		for (auto pgn : this->polygons_with_holes()) {
			if (!pgn.outer_boundary().is_simple()) {
				throw std::runtime_error("Encountered non-simple polygon");
			}
			if (pgn.outer_boundary().is_clockwise_oriented()) {
				pgn.outer_boundary().reverse_orientation();
			}
			for (auto& hole : pgn.holes()) {
				if (!hole.is_simple()) {
					throw std::runtime_error("Encountered non-simple polygon");
				}
				if (hole.is_counterclockwise_oriented()) {
					hole.reverse_orientation();
				}
			}
			polygonSet.symmetric_difference(pgn);
		}
		return polygonSet;
	}

	MultipolygonWithHoles() = default;

	MultipolygonWithHoles(Polygon<K, Container_> polygon) {
		this->add_polygon(std::move(polygon));
	}

	MultipolygonWithHoles(PolygonWithHoles<K> polygon) {
		this->add_polygon_with_holes(std::move(polygon));
	}

};

template <typename KernelOut, typename KernelIn>
MultipolygonWithHoles<KernelOut> convert_kernel(const MultipolygonWithHoles<KernelIn>& v) {
	MultipolygonWithHoles<KernelOut> result;
	for (const auto& p : v.polygons_with_holes()) {
		result.add_polygon_with_holes(convert_kernel<KernelOut>(p));
	}
	return result;
}


template <typename KernelIn>
MultipolygonWithHoles<Exact> pretendExact(const MultipolygonWithHoles<KernelIn>& v) {
	return convert_kernel<Exact>(v);
}

template <typename KernelIn>
MultipolygonWithHoles<Inexact> approximate(const MultipolygonWithHoles<KernelIn>& v) {
	return convert_kernel<Inexact>(v);
}
}