#ifndef CARTOCROW_CIRCLE_TANGENTS_H
#define CARTOCROW_CIRCLE_TANGENTS_H

#include "cs_types.h"
#include "cartocrow/core/polyline.h"

namespace cartocrow {
/// A circle with rational radius and rational center coordinates.
struct RationalRadiusCircle {
    Point<Exact> center;
    Number<Exact> radius;
	Circle<Exact> circle() const;
};

/// Approximate a circle by one with rational radius.
RationalRadiusCircle approximateRadiusCircle(const Circle<Exact>& circle);

/// Circle bitangents.
/// No bitangents are returned if the circles overlap or are nested within each-other.
/// \p inner specifies whether inner or outer bitangents are computed.
/// The returned segments are directed from c1 to c2.
/// The first segment of the pair has the property that c1 lies to the left of it.
std::optional<std::pair<Segment<Inexact>, Segment<Inexact>>>
bitangents(const Circle<Inexact>& c1, const Circle<Inexact>& c2, bool inner= false);

/// Tangents to a circle through a point.
/// No tangents are returned if the point lies on or inside the circle.
/// The returned segments are directed from p to c.
/// The first segment of the pair has the property that c lies to the left of it.
std::optional<std::pair<Segment<Inexact>, Segment<Inexact>>>
tangents(const Point<Inexact>& p, const Circle<Inexact>& c);

/// Tangents to a circle through a point.
/// No tangents are returned if the point lies on or inside the circle.
/// The returned segments are directed from c to p.
/// The first segment of the pair has the property that c lies to the left of it.
std::optional<std::pair<Segment<Inexact>, Segment<Inexact>>>
tangents(const Circle<Inexact>& c, const Point<Inexact>& p);

/// Compute the endpoints of bitangents between two circles with rational radii.
/// The endpoints may have irrational coordinates.
/// No tangent endpoints are returned if the circles overlap or are nested within each-other.
/// \p inner specifies whether inner or outer bitangents are computed.
/// The first of each point pair representing one tangent lies on c1.
/// The first tangent of the pair has the property that c1 lies to the left of it.
/// \sa rationalBitangents
std::optional<std::pair<std::pair<OneRootPoint, OneRootPoint>,std::pair<OneRootPoint, OneRootPoint>>>
bitangentPoints(const RationalRadiusCircle& c1, const RationalRadiusCircle& c2, bool inner= false);

/// Endpoints of tangents to a circle through a point.
/// The endpoints may have irrational coordinates.
/// No tangent endpoints are returned if the point lies on or inside the circle.
/// The first point of the pair has the property that the tangent at that point, directed towards c, has c to the left of it.
std::optional<std::pair<OneRootPoint, OneRootPoint>>
tangentPoints(const Point<Exact>& p, const RationalRadiusCircle& c);

/// Endpoints of tangents to a circle through a point.
/// The endpoints may have irrational coordinates.
/// No tangent endpoints are returned if the point lies on or inside the circle.
/// The first point of the pair has the property that the tangent at that point, directed towards p, has c to the left of it.
std::optional<std::pair<OneRootPoint, OneRootPoint>>
tangentPoints(const RationalRadiusCircle& c, const Point<Exact>& p);

class RationalTangent {
  public:
	RationalTangent() = default;
	RationalTangent(Segment<Exact> seg) : variant(seg) {}
	RationalTangent(Segment<Exact> seg1, Segment<Exact> seg2) : variant(std::pair(seg1, seg2)) {}
	RationalTangent(std::pair<Segment<Exact>, Segment<Exact>> segs) : variant(segs) {}
	RationalTangent(std::variant<Segment<Exact>, std::pair<Segment<Exact>, Segment<Exact>>> variant) : variant(std::move(variant)) {}
	std::variant<Segment<Exact>, std::pair<Segment<Exact>, Segment<Exact>>> variant;
	Polyline<Exact> polyline() const;
	Point<Exact> source() const;
	Point<Exact> target() const;
	RationalTangent opposite() const;
};

/// Compute approximate bitangents between circles in Exact representation with rational radius.
/// The bitangents are approximate in the sense that their endpoints are 'snapped' to points with rational coordinates,
/// so that the corresponding segments can be represented in circle-segment traits geometries.
/// Each tangent consists of one or two line segments for which the following properties hold.
/// - The endpoints of the bitangents lie exactly on the corresponding circles.
/// - The circles and bitangents are interior-disjoint.
/// - The line segments are tangent to circles at their endpoints.
std::optional<std::pair<RationalTangent, RationalTangent>>
rationalBitangents(const RationalRadiusCircle& c1, const RationalRadiusCircle& c2, bool inner= false);

/// Compute approximate tangents to a circle in Exact representation with rational radius.
/// The tangents are approximate in the sense that their endpoints are 'snapped' to points with rational coordinates,
/// so that the corresponding segments can be represented in circle-segment traits geometries.
/// Each tangent consists of one or two line segments for which the following properties hold.
/// - One endpoint of the tangent lies exactly on the circle and one exactly equals the given point.
/// - The circles and tangents are interior-disjoint.
/// - The line segments are tangent to the circle.
std::optional<std::pair<RationalTangent, RationalTangent>>
rationalTangents(const Point<Exact>& p, const RationalRadiusCircle& c);

/// Compute approximate tangents to a circle in Exact representation with rational radius.
/// The tangents are approximate in the sense that their endpoints are 'snapped' to points with rational coordinates,
/// so that the corresponding segments can be represented in circle-segment traits geometries.
/// Each tangent consists of one or two line segments for which the following properties hold.
/// - One endpoint of the tangent lies exactly on the circle and one exactly equals the given point.
/// - The circles and tangents are interior-disjoint.
/// - The line segments are tangent to the circle.
std::optional<std::pair<RationalTangent, RationalTangent>>
rationalTangents(const RationalRadiusCircle& c, const Point<Exact>& p);
}
#endif //CARTOCROW_CIRCLE_TANGENTS_H
