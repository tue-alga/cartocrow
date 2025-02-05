#include "approximate_convex_hull_of_disks.h"
#include "cartocrow/core/cs_curve_helpers.h"
#include "cartocrow/core/cs_polygon_helpers.h"
#include "cartocrow/core/circle_tangents.h"

#include <CGAL/Apollonius_site_2.h>
#include <CGAL/Apollonius_graph_2.h>
#include <CGAL/Apollonius_graph_traits_2.h>
#include <gmpxx.h>

namespace cartocrow {
typedef CGAL::Apollonius_graph_traits_2<Exact> AT;
typedef CGAL::Apollonius_graph_2<AT>           Apollonius;
typedef Apollonius::Site_2                     ASite;

std::vector<RationalRadiusCircle> circlesOnConvexHull(const std::vector<RationalRadiusCircle>& circles) {
	if (circles.size() == 1) {
		return {circles.front()};
	}

	Apollonius apo;
	std::unordered_map<Apollonius::Vertex_handle, RationalRadiusCircle> vToCircle;
	for (const auto& c : circles) {
		auto vh = apo.insert(ASite(c.center, c.radius));
		vToCircle[vh] = c;
	}
	if (apo.number_of_vertices() == 1) {
		auto site = apo.finite_vertices_begin()->site();
		return {{site.point(), site.weight()}};
	}
	auto circ = apo.incident_vertices(apo.infinite_vertex());
	auto curr = circ;
	std::vector<RationalRadiusCircle> hullCircles;
	do {
		hullCircles.push_back(vToCircle[curr]);
	} while (++curr != circ);

	std::reverse(hullCircles.begin(), hullCircles.end());

	return hullCircles;
}

CSPolygon approximateConvexHull(const std::vector<Circle<Exact>>& circles) {
    if (circles.size() == 1) {
        auto circlePolygon = circleToCSPolygon(circles[0]);
        if (circlePolygon.orientation() == CGAL::CLOCKWISE) {
            circlePolygon.reverse_orientation();
        }
        return circlePolygon;
    }

    std::vector<RationalRadiusCircle> rrCircles;
    for (const auto& c : circles) {
        rrCircles.push_back(approximateRadiusCircle(c));
    }
    return approximateConvexHull(rrCircles);
}

CSPolygon approximateConvexHull(const std::vector<RationalRadiusCircle>& rrCircles) {
	// todo: approximating circle radii may cause problems when two circles overlap in a single point and one is contained in the other.
	// solution? filter out any circle that is contained in another, before approximating the radii.
	auto hullCircles = circlesOnConvexHull(rrCircles);
	if (hullCircles.size() == 1) {
        auto circlePolygon = circleToCSPolygon(Circle<Exact>(hullCircles[0].center, hullCircles[0].radius));
        if (circlePolygon.orientation() == CGAL::CLOCKWISE) {
            circlePolygon.reverse_orientation();
        }
        return circlePolygon;
	}

	std::vector<std::vector<Segment<Exact>>> tangents;

	for (int i = 0; i < hullCircles.size(); ++i) {
		auto& c1 = hullCircles[i];
		auto& c2 = hullCircles[(i + 1) % hullCircles.size()];
		auto segOrPair = rationalTangents(c1, c2)->first;
		std::vector<Segment<Exact>> segs;
		if (segOrPair.index() == 0) {
			segs.push_back(std::get<Segment<Exact>>(segOrPair));
		} else {
			auto pair = std::get<std::pair<Segment<Exact>, Segment<Exact>>>(segOrPair);
			segs.push_back(pair.first);
			segs.push_back(pair.second);
		}
		tangents.push_back(segs);
	}

	std::vector<CSXMCurve> xm_curves;
	for (int i = 0; i < hullCircles.size(); ++i) {
        auto& c2 = hullCircles[(i + 1) % hullCircles.size()];
		auto& t1 = tangents[i];
		auto& t2 = tangents[(i + 1) % tangents.size()];
		for (const auto& piece : t1) {
			CSCurve curve(piece);
			curveToXMonotoneCurves(curve, std::back_inserter(xm_curves));
		}
		OneRootPoint t1End(t1.back().target().x(), t1.back().target().y());
		OneRootPoint t2Start(t2.front().source().x(), t2.front().source().y());
		CSCurve arc(Circle<Exact>(c2.center, c2.radius * c2.radius), t1End, t2Start);
		curveToXMonotoneCurves(arc, std::back_inserter(xm_curves));
	}

	return {xm_curves.begin(), xm_curves.end()};
}
}