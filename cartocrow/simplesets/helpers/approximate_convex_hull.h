#ifndef CARTOCROW_APPROXIMATE_CONVEX_HULL_H
#define CARTOCROW_APPROXIMATE_CONVEX_HULL_H

#include "../types.h"
#include <CGAL/Apollonius_site_2.h>
#include <CGAL/Apollonius_graph_2.h>
#include <CGAL/Apollonius_graph_traits_2.h>
#include <gmpxx.h>

namespace cartocrow::simplesets {
typedef CGAL::Apollonius_graph_traits_2<Exact> AT;
typedef CGAL::Apollonius_graph_2<AT>           Apollonius;
typedef Apollonius::Site_2                     ASite;


struct RationalRadiusCircle {
	Point<Exact> center;
	Number<Exact> radius;
};

Segment<Inexact> tangent(const Circle<Inexact>& c1, const Circle<Inexact>& c2);

std::pair<Point<Inexact>, Point<Inexact>>
tangentPoints(const Circle<Inexact>& c1, const Circle<Inexact>& c2);

std::pair<CSTraits::Point_2, CSTraits::Point_2>
tangentPoints(const RationalRadiusCircle& c1, const RationalRadiusCircle& c2);

CSTraits::Point_2 closestOnCircle(const Circle<Exact>& circle, const Point<Inexact>& point);

std::variant<Segment<Exact>, std::pair<Segment<Exact>, Segment<Exact>>>
algebraicCircleTangentToRationalSegments(const CSTraits::Point_2& p1, const CSTraits::Point_2& p2,
                                         const Circle<Exact>& c1, const Circle<Exact>& c2);

CSPolygon approximateConvexHull(const std::vector<Circle<Exact>>& circles);
}

#endif //CARTOCROW_APPROXIMATE_CONVEX_HULL_H
