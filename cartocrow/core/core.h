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

#ifndef CARTOCROW_CORE_CORE_H
#define CARTOCROW_CORE_CORE_H

#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Bbox_2.h>
#include <CGAL/Circle_2.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Line_2.h>
#include <CGAL/Point_2.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_set_2.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/Segment_2.h>
#include <CGAL/Vector_2.h>
#include <CGAL/number_utils.h>

namespace cartocrow {

/// CGAL kernel for exact constructions (uses an exact number type).
using Exact = CGAL::Exact_predicates_exact_constructions_kernel;
/// CGAL kernel for inexact constructions.
using Inexact = CGAL::Exact_predicates_inexact_constructions_kernel;

/// The number type used for coordinates.
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
/// A point set with polygonal boundaries. See \ref CGAL::Polygon_set_2.
template <class K> using PolygonSet = CGAL::Polygon_set_2<K>;

/// Axis-aligned bounding box with inexact coordinates. See \ref
/// CGAL::Polygon_2.
using Box = CGAL::Bbox_2;

/// An arrangement of objects in the plane.
template <class K> using Arrangement = CGAL::Arrangement_2<CGAL::Arr_segment_traits_2<K>>;

/// An epsilon value.
/**
 * Obviously this should be used only for non-exact computation. For this
 * reason we provide only a definition for `Number<Inexact>` and not for
 * `Number<Exact>`.
 */
constexpr const Number<Inexact> M_EPSILON = 0.0000001;

/// Converts a number from exact representation to an approximation in inexact
/// representation.
Number<Inexact> approximate(const Number<Exact>& p);
/// Converts a point from exact representation to an approximation in inexact
/// representation.
Point<Inexact> approximate(const Point<Exact>& p);
/// Converts a vector from exact representation to an approximation in inexact
/// representation.
Vector<Inexact> approximate(const Vector<Exact>& p);
/// Converts a circle from exact representation to an approximation in inexact
/// representation.
Circle<Inexact> approximate(const Circle<Exact>& p);
/// Converts a line from exact representation to an approximation in inexact
/// representation.
Line<Inexact> approximate(const Line<Exact>& p);
/// Converts a line segment from exact representation to an approximation in
/// inexact representation.
Segment<Inexact> approximate(const Segment<Exact>& p);
/// Converts a polygon from exact representation to an approximation in inexact
/// representation.
Polygon<Inexact> approximate(const Polygon<Exact>& p);
/// Converts a polygon with holes from exact representation to an approximation
/// in inexact representation.
PolygonWithHoles<Inexact> approximate(const PolygonWithHoles<Exact>& p);
/// Converts a polygon set from exact representation to an approximation in
/// inexact representation.
PolygonSet<Inexact> approximate(const PolygonSet<Exact>& p);

/// Returns the area of a polygon with holes.
/// TODO: move to more logical place
template <class K> Number<K> area(PolygonWithHoles<K> polygon) {
	Number<K> a = polygon.outer_boundary().area();
	for (const auto &h : polygon.holes()) a -= h.area();
	return a;  // = outer area minus area of each hole
}

/// An RGB color. Used for storing the color of elements to be drawn.
struct Color {
	/// Red component (integer 0-255).
	int r;
	/// Green component (integer 0-255).
	int g;
	/// Blue component (integer 0-255).
	int b;

	/// Returns a new color that is darker (\f$0 \le f < 1\f$) or lighter
	/// (\f$1 < f \le 2\f$).
	Color shaded(double f) const;
};

/// Wraps the given number \f$n\f$ to the interval \f$[a, b)\f$.
/**
 * The returned number \f$r\f$ is \f$n + k \cdot (b - a)\f$ for \f$k \in
 * \mathbb{Z}\f$ such that \f$r \in [a, b)\f$.
 */
template <class K> Number<K> wrap(Number<K> n, Number<K> a, Number<K> b) {
	Number<K> constrained = n;
	Number<K> interval_size = b - a;
	while (constrained < a)
		constrained += interval_size;
	while (a + interval_size <= constrained)
		constrained -= interval_size;
	return constrained;
}

/// Wraps the given number \f$n\f$ to the interval \f$(a, b]\f$.
/**
 * The returned number \f$r\f$ is \f$n + k \cdot (b - a)\f$ for \f$k \in
 * \mathbb{Z}\f$ such that \f$r \in (a, b]\f$.
 */
template <class K> Number<K> wrapUpper(Number<K> n, Number<K> a, Number<K> b) {
	Number<K> constrained = n;
	Number<K> interval_size = b - a;
	while (constrained <= a)
		constrained += interval_size;
	while (a + interval_size < constrained)
		constrained -= interval_size;
	return constrained;
}

/// Wraps the given number \f$\alpha\f$ to the interval \f$[\beta, \beta +
/// 2\pi)\f$.
Number<Inexact> wrapAngle(Number<Inexact> alpha, Number<Inexact> beta = 0);
/// Wraps the given number \f$\alpha\f$ to the interval \f$(\beta, \beta +
/// 2\pi]\f$.
Number<Inexact> wrapAngleUpper(Number<Inexact> alpha, Number<Inexact> beta = 0);

/// \f$2 \pi\f$, defined here for convenience.
constexpr Number<Inexact> M_2xPI = M_PI * 2;

} // namespace cartocrow

#endif //CARTOCROW_CORE_CORE_H
