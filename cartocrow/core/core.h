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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 07-11-2019
*/

#ifndef CARTOCROW_CORE_CGAL_TYPES_H
#define CARTOCROW_CORE_CGAL_TYPES_H

#include <CGAL/Arr_extended_dcel.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Circle_2.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Line_2.h>
#include <CGAL/Point_2.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/Segment_2.h>
#include <CGAL/Vector_2.h>

namespace cartocrow {

/// CGAL kernel for exact constructions (uses an exact number type).
using Exact = CGAL::Exact_predicates_exact_constructions_kernel;
/// CGAL kernel for inexact constructions.
using Inexact = CGAL::Exact_predicates_inexact_constructions_kernel;

/// The exact number type used for coordinates.
template <class K> using Number = typename K::FT;

/// A point in the plane. See \ref CGAL::Point_2.
template <class K> using Point = CGAL::Point_2<K>;
/// A vector in the plane. See \ref CGAL::Vector_2.
template <class K> using Vector = CGAL::Vector_2<K>;
/// A circle in the plane. See \ref CGAL::Circle_2.
template <class K> using Circle = CGAL::Circle_2<K>;
/// A line in the plane. See \ref CGAL::Line_2.
template <class K> using Line = CGAL::Line_2<K>;
/// A line segment in the plane. See \ref CGAL::Segment_2.
template <class K> using Segment = CGAL::Segment_2<K>;

/// A polygon in the plane. See \ref CGAL::Polygon_2.
template <class K> using Polygon = CGAL::Polygon_2<K>;
/// A polygon with holes in the plane. See \ref CGAL::Polygon_2.
template <class K> using PolygonWithHoles = CGAL::Polygon_with_holes_2<K>;

namespace detail {
template <class K> using DCELTraits = CGAL::Arr_segment_traits_2<K>;

template <class K, typename TVertexData, typename TEdgeData, typename TFaceData>
using MapArrangement =
    CGAL::Arrangement_2<DCELTraits<K>,
                        CGAL::Arr_extended_dcel<DCELTraits<K>, TVertexData, TEdgeData, TFaceData>>;
} // namespace detail

/// A map consisting of polygonal regions. See \ref CGAL::Polygon_2.
template <class K> using Map = detail::MapArrangement<K, int, int, int>;

/// An epsilon value.
/**
 * Obviously this should be used only for non-exact computation. For this
 * reason we provide only a definition for `Number<Inexact>` and not for
 * `Number<Exact>`.
 */
constexpr const Number<Inexact> EPSILON = 0.0000001;

/// An RGB color. Used for storing the color of elements to be drawn.
struct Color {
	/// Red component (integer 0-255).
	int r;
	/// Green component (integer 0-255).
	int g;
	/// Blue component (integer 0-255).
	int b;
};

} // namespace cartocrow

#endif //CARTOCROW_CORE_CGAL_TYPES_H
