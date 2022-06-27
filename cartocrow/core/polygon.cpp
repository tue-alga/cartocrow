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

#include "polygon.h"

#include <CGAL/Kernel/global_functions_2.h>
#include <CGAL/Origin.h>
#include <glog/logging.h>
#include <stdexcept>

namespace cartocrow {

Point<Exact> centroid(const Polygon<Exact>& polygon) {
	if (polygon.size() == 1) {
		return polygon[0];
	}

	Number<Exact> area = polygon.area();
	if (area == 0) {
		throw std::runtime_error("Centroid cannot be computed for polygons of area 0");
	}

	Vector<Exact> sum(0, 0);
	for (Polygon<Exact>::Edge_const_iterator edge_iter = polygon.edges_begin();
	     edge_iter != polygon.edges_end(); ++edge_iter) {
		const Number<Exact> weight = edge_iter->source().x() * edge_iter->target().y() -
		                             edge_iter->target().x() * edge_iter->source().y();
		sum += weight * (edge_iter->source() - CGAL::ORIGIN);
		sum += weight * (edge_iter->target() - CGAL::ORIGIN);
	}

	return CGAL::ORIGIN + sum / (6 * area);
}

Point<Exact> centroid(const PolygonWithHoles<Exact>& polygon) {
	Number<Exact> area = CGAL::abs(polygon.outer_boundary().area());
	Vector<Exact> sum = area * (centroid(polygon.outer_boundary()) - CGAL::ORIGIN);
	Number<Exact> areaSum = area;
	for (PolygonWithHoles<Exact>::Hole_const_iterator hole_iter = polygon.holes_begin();
	     hole_iter != polygon.holes_end(); ++hole_iter) {
		Number<Exact> area = CGAL::abs(hole_iter->area());
		sum -= area * (centroid(*hole_iter) - CGAL::ORIGIN);
		areaSum -= area;
	}
	return CGAL::ORIGIN + sum / areaSum;
}

} // namespace cartocrow
