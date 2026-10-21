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

#include "multipolygon_with_holes.h"

namespace cartocrow {
template <class Container_>
MultipolygonWithHoles<Inexact, Container_> approximate(const MultipolygonWithHoles<Exact, Container_>& pgs) {
	MultipolygonWithHoles<Inexact, Container_> approximated;
	for (const PolygonWithHoles<Exact, Container_>& pgn : pgs.polygons_with_holes()) {
		approximated.add_polygon_with_holes(cartocrow::approximate(pgn));
	}
	return approximated;
}

template <class Container_>
MultipolygonWithHoles<Exact, Container_> pretendExact(const MultipolygonWithHoles<Inexact, Container_>& pgs) {
	MultipolygonWithHoles<Exact, Container_> exact;
	for (const auto& pgn : pgs.polygons_with_holes()) {
		exact.add_polygon_with_holes(cartocrow::pretendExact(pgn));
	}
	return exact;
}
}