#ifndef CARTOCROW_APPROXIMATE_CONVEX_HULL_OF_DISKS_H
#define CARTOCROW_APPROXIMATE_CONVEX_HULL_OF_DISKS_H

#include "cs_types.h"
#include "circle_tangents.h"

namespace cartocrow {
/// Given circles with rational radii, returns the circles that are part of their convex hull.
/// \pre the circle centers are distinct.
std::vector<RationalRadiusCircle> circlesOnConvexHull(const std::vector<RationalRadiusCircle>& circles);
/// Return the approximate convex hull, oriented counter-clockwise, of the provided circles.
/// The convex hull is approximate in the same sense that tangents between rational radius circles
/// are approximate. That is, the returned convex hull is a superset of the exact convex hull;
/// tangents may consist of two line segments each tangent to one circle, which meet at a point
/// outside the exact convex hull.
/// \pre the circle centers are distinct.
/// \sa rationalTangents
CSPolygon approximateConvexHull(const std::vector<RationalRadiusCircle>& rrCircles);
/// Return the approximate convex hull, oriented counter-clockwise, of the provided circles.
/// The radii are first approximated by rational numbers; then approximate tangents are computed.
/// If the circles have rational radius use the overloaded function that takes objects of type RationalRadiusCircle instead.
/// \pre the circle centers are distinct.
/// \sa rationalTangents
CSPolygon approximateConvexHull(const std::vector<Circle<Exact>>& circles);
}

#endif //CARTOCROW_APPROXIMATE_CONVEX_HULL_OF_DISKS_H
