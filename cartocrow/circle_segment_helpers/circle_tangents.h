#ifndef CARTOCROW_CIRCLE_TANGENTS_H
#define CARTOCROW_CIRCLE_TANGENTS_H

#include "cs_types.h"

namespace cartocrow {
/// A circle with rational radius and rational center coordinates.
struct RationalRadiusCircle {
    Point<Exact> center;
    Number<Exact> radius;
};

/// Approximate a circle by one with rational radius.
RationalRadiusCircle approximateRadiusCircle(const Circle<Exact>& circle);

/// Tangents between circles.
/// No tangents are returned if the circles overlap or are nested within each-other.
/// \p inner specifies whether inner or outer tangents are computed.
/// The returned segments are directed from c1 to c2.
/// The first segment of the pair has the property that c1 lies to the left of it.
std::optional<std::pair<Segment<Inexact>, Segment<Inexact>>>
tangents(const Circle<Inexact>& c1, const Circle<Inexact>& c2, bool inner=false);

/// Compute the endpoints of tangents between two circles with rational radii.
/// The endpoints may have irrational coordinates.
/// No tangent endpoints are returned if the circles overlap or are nested within each-other.
/// \p inner specifies whether inner or outer tangents are computed.
/// The first of each point pair representing one tangent lies on c1.
/// The first tangent of the pair has the property that c1 lies to the left of it.
/// \sa rationalTangents
std::optional<std::pair<std::pair<ArrCSTraits::Point_2, ArrCSTraits::Point_2>,std::pair<ArrCSTraits::Point_2, ArrCSTraits::Point_2>>>
tangentPoints(const RationalRadiusCircle& c1, const RationalRadiusCircle& c2, bool inner=false);

using RationalTangent = std::variant<Segment<Exact>, std::pair<Segment<Exact>, Segment<Exact>>>;

/// Opposite of a rational tangent.
RationalTangent opposite(const RationalTangent& rt);

/// Compute approximate tangents between circles in Exact representation with rational radius.
/// The tangents are approximate in the sense that their endpoints are 'snapped' to points with rational coordinates,
/// so that the corresponding Segments can be represented in circle-segment traits geometries.
/// The result is one or two line segments that have the properties:
/// - The endpoints of the tangents lie exactly on the corresponding circles.
/// - Circles and their corresponding tangents are interior-disjoint.
/// - The line segments returned are in tangent to circles at their endpoints.
std::optional<std::pair<RationalTangent, RationalTangent>>
rationalTangents(const RationalRadiusCircle& c1, const RationalRadiusCircle& c2, bool inner=false);
}
#endif //CARTOCROW_CIRCLE_TANGENTS_H
