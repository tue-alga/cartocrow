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

#include "polyline.h"

// Mirrors CGAL's Multipolygon_with_holes and Polygon_set
namespace cartocrow {
template<class K>
struct MultiPolyline {
   std::vector<Polyline<K>> polylines;

   MultiPolyline<K> transform(const CGAL::Aff_transformation_2<K>& trans) const {
       MultiPolyline<K> transformed;
       for (const auto& pl : polylines) {
           transformed.polylines.push_back(pl.transform(trans));
       }
       return transformed;
   }

   Box bbox() const {
       return CGAL::bbox_2(polylines.begin(), polylines.end());
   }
};

template <typename KernelOut, typename KernelIn>
MultiPolyline<KernelOut> convert_kernel(const MultiPolyline<KernelIn>& v) {
	MultiPolyline<KernelOut> result;
	for (const auto& p : v.polylines) {
        result.polylines.push_back(convert_kernel<KernelOut>(*p));
	}
	return result;
}

}
