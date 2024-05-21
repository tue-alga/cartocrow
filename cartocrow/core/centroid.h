/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
Copyright (C) 2021  Netherlands eScience Center and TU Eindhoven

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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 05-12-2019
*/

#ifndef CARTOCROW_CORE_POLYGON_H
#define CARTOCROW_CORE_POLYGON_H

#include "core.h"

#include <CGAL/Kernel/global_functions_2.h>
#include <CGAL/Origin.h>
#include <numeric>
#include <stdexcept>

namespace cartocrow {

/// Computes the centroid of the given polygon.
///
/// This method throws if the polygon has area 0, in which case the centroid
/// would be ill-defined.
template <class K> Point<K> centroid(const Polygon<K>& polygon) {
	if (polygon.size() == 1) {
		return polygon[0];
	}

	Number<K> area = polygon.area();
	if (area == 0) {
		throw std::runtime_error("Centroid cannot be computed for polygons of area 0");
	}

	Vector<K> sum(0, 0);
	for (typename Polygon<K>::Edge_const_iterator edge_iter = polygon.edges_begin();
	     edge_iter != polygon.edges_end(); ++edge_iter) {
		const Number<K> weight = edge_iter->source().x() * edge_iter->target().y() -
		                         edge_iter->target().x() * edge_iter->source().y();
		sum += weight * (edge_iter->source() - CGAL::ORIGIN);
		sum += weight * (edge_iter->target() - CGAL::ORIGIN);
	}

	return CGAL::ORIGIN + sum / (6 * area);
}

/// Computes the centroid of the given polygon with holes.
template <class K> Point<K> centroid(const PolygonWithHoles<K>& polygon) {
	Number<K> area = CGAL::abs(polygon.outer_boundary().area());
	Vector<K> sum = area * (centroid(polygon.outer_boundary()) - CGAL::ORIGIN);
	Number<K> areaSum = area;
	for (typename PolygonWithHoles<K>::Hole_const_iterator hole_iter = polygon.holes_begin();
	     hole_iter != polygon.holes_end(); ++hole_iter) {
		Number<K> area = CGAL::abs(hole_iter->area());
		sum -= area * (centroid(*hole_iter) - CGAL::ORIGIN);
		areaSum -= area;
	}
	return CGAL::ORIGIN + sum / areaSum;
}

/// Computes the centroid of the given polygon set.
template <class K> Point<K> centroid(const PolygonSet<K>& polygon) {
	std::vector<PolygonWithHoles<K>> polygons;
	polygon.polygons_with_holes(std::back_inserter(polygons));
	Vector<K> sum(0, 0);
	Number<K> areaSum = 0;
	for (const PolygonWithHoles<K>& p : polygons) {
		Number<K> area = CGAL::abs(p.outer_boundary().area());
		for (typename PolygonWithHoles<K>::Hole_const_iterator hole_iter = p.holes_begin();
		     hole_iter != p.holes_end(); ++hole_iter) {
			area -= CGAL::abs(hole_iter->area());
		}
		sum += area * (centroid(p) - CGAL::ORIGIN);
		areaSum += area;
	}
	return CGAL::ORIGIN + sum / areaSum;
}

} // namespace cartocrow

#endif //CARTOCROW_CORE_POLYGON_H
