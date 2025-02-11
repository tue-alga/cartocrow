#include "drawing_algorithm.h"

#include "cartocrow/circle_segment_helpers/approximate_convex_hull_of_disks.h"
#include "cartocrow/circle_segment_helpers/cavc_helpers.h"
#include "cartocrow/circle_segment_helpers/cs_curve_helpers.h"
#include "cartocrow/circle_segment_helpers/cs_polygon_helpers.h"
#include "cartocrow/circle_segment_helpers/cs_polyline_helpers.h"
#include "cartocrow/circle_segment_helpers/cs_render_helpers.h"
#include "cartocrow/circle_segment_helpers/poly_line_gon_intersection.h"
#include "cartocrow/core/arrangement_helpers.h"
#include "cartocrow/renderer/ipe_renderer.h"
#include "grow_circles.h"
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Boolean_set_operations_2/Gps_polygon_validation.h>
#include <utility>

using namespace cartocrow::renderer;

namespace cartocrow::simplesets {
// todo: deal with holes
CSPolygon face_to_polygon(const Face& face) {
	assert(face.number_of_holes() == 0);
	return ccb_to_general_polygon<ArrCSTraits>(face.outer_ccb());
}

CSXMCurve make_x_monotone(const Segment<Exact>& segment) {
	ArrCSTraits traits;
	auto make_x_monotone = traits.make_x_monotone_2_object();
	std::vector<boost::variant<ArrCSTraits::Point_2, CSXMCurve>> curves_and_points;
	make_x_monotone(segment, std::back_inserter(curves_and_points));
	std::vector<CSXMCurve> curves;

	// There should not be any isolated points
	for (auto kinda_curve : curves_and_points) {
		assert(kinda_curve.which() == 1);
		auto curve = boost::get<CSXMCurve>(kinda_curve);
		curves.push_back(curve);
	}

	assert(curves.size() == 1);
	return curves[0];
}

Point<Exact> get_approx_point_on_boundary(const Face& face) {
	auto curve = face.outer_ccb()->curve();
	Rectangle<Exact> bbox = curve.bbox();
	Rectangle<Exact> rect({bbox.xmin() - 1, bbox.ymin() - 1}, {bbox.xmax() + 1, bbox.ymax() + 1});
	Point<Exact> approx_source = pretendExact(approximateOneRootPoint(curve.source()));
	Point<Exact> approx_target = pretendExact(approximateOneRootPoint(curve.target()));
	Point<Exact> middle = CGAL::midpoint(approx_source, approx_target);
	if (curve.is_linear()) {
		return middle;
	} else {
		assert(curve.is_circular());
		Circle<Exact> circle = curve.supporting_circle();
		Line<Exact> l(approx_source, approx_target);
		Line<Exact> pl = l.perpendicular(middle);
		auto inter = CGAL::intersection(pl, rect);
		assert(inter.has_value());
		assert(inter->which() == 1);
		Segment<Exact> seg = boost::get<Segment<Exact>>(*inter);
//		CGAL::intersection(pl, circle);
		ArrCSTraits traits;
		auto make_x_monotone = traits.make_x_monotone_2_object();
		std::vector<boost::variant<ArrCSTraits::Point_2, CSXMCurve>> curves_and_points;
//		make_x_monotone(circle, std::back_inserter(curves_and_points));
		make_x_monotone(seg, std::back_inserter(curves_and_points));
		std::vector<CSXMCurve> curves;

		// There should not be any isolated points
		for (auto kinda_curve : curves_and_points) {
			auto curve = boost::get<CSXMCurve>(kinda_curve);
			curves.push_back(curve);
		}

		typedef std::pair<ArrCSTraits::Point_2, unsigned int> Intersection_point;
		typedef boost::variant<Intersection_point, CSXMCurve> Intersection_result;
		std::vector<Intersection_result> intersection_results;
		assert(curves.size() == 1);
		curve.intersect(curves[0], std::back_inserter(intersection_results));

		std::vector<ArrCSTraits::Point_2> intersection_pts;
		for (const auto& result : intersection_results) {
			assert(result.which() == 0);
			intersection_pts.push_back(boost::get<Intersection_point>(result).first);
		}

		assert(intersection_pts.size() == 1);
		return pretendExact(approximateOneRootPoint(intersection_pts[0]));
	}
}

Point<Exact> get_point_in(const Face& face) {
	auto poly = face_to_polygon(face);
	Rectangle<Exact> bbox = poly.bbox();
	Rectangle<Exact> rect({bbox.xmin() - 1, bbox.ymin() - 1}, {bbox.xmax() + 1, bbox.ymax() + 1});
	Point<Exact> point_outside(rect.xmin(), rect.ymin());
	Point<Exact> approx_point_on_boundary = get_approx_point_on_boundary(face);
	Line<Exact> line(point_outside, approx_point_on_boundary);
	auto line_inter_box = CGAL::intersection(rect, line);
	auto seg = boost::get<Segment<Exact>>(*line_inter_box);
	auto seg_curve = make_x_monotone(seg);
	std::vector<ArrCSTraits::Point_2> intersection_pts;
	for (auto cit = poly.curves_begin(); cit != poly.curves_end(); cit++) {
		auto curve = *cit;
		typedef std::pair<ArrCSTraits::Point_2, unsigned int> Intersection_point;
		typedef boost::variant<Intersection_point, CSXMCurve> Intersection_result;
		std::vector<Intersection_result> intersection_results;
		curve.intersect(seg_curve, std::back_inserter(intersection_results));

		for (const auto& result : intersection_results) {
			assert(result.which() == 0);
			intersection_pts.push_back(boost::get<Intersection_point>(result).first);
		}
	}

	std::sort(intersection_pts.begin(), intersection_pts.end(), [&seg](const auto& pt1, const auto& pt2) {
		Vector<Exact> v = seg.supporting_line().to_vector();
		return v * (pretendExact(approximateOneRootPoint(pt1)) - pretendExact(approximateOneRootPoint(pt2))) < 0;
	});
	intersection_pts.erase(std::unique(intersection_pts.begin(), intersection_pts.end()), intersection_pts.end());

	Point<Exact> approx_source;
	Point<Exact> approx_target;
	if (intersection_pts.size() >= 2) {
		approx_source = pretendExact(approximateOneRootPoint(intersection_pts[0]));
		approx_target = pretendExact(approximateOneRootPoint(intersection_pts[1]));
	} else {
		approx_source = pretendExact(approximateOneRootPoint(intersection_pts[0]));
		approx_target = approx_point_on_boundary;
	}

	return midpoint(Segment<Exact>(approx_source, approx_target));
}

std::ostream& operator<<(std::ostream& out, const Order& o) {
	return out << to_string(o);
}

std::ostream& operator<<(std::ostream& out, const Relation& r) {
	return out << r.left << " " << r.ordering
	           << (r.preference != r.ordering ? "(" + to_string(r.preference) + ")" : "")
	           << " " << r.right;
}

bool operator==(const Relation& lhs, const Relation& rhs) {
	return lhs.left == rhs.left && lhs.right == rhs.right && lhs.preference == rhs.preference && lhs.ordering == rhs.ordering;
}

bool operator==(const Hyperedge& lhs, const Hyperedge& rhs) {
	return lhs.relations == rhs.relations && lhs.origins == rhs.origins;
}

DilatedPatternDrawing::DilatedPatternDrawing(const Partition& partition, const GeneralSettings& gs, const ComputeDrawingSettings& cds)
	: m_gs(gs), m_cds(cds) {
	for (const auto& p : partition) {
		m_dilated.emplace_back(*p, gs.dilationRadius());
	}

	std::vector<ArrCSTraits::Curve_2> curves;
	std::vector<std::pair<ArrCSTraits::Curve_2, int>> curves_data;
	for (int i = 0; i < m_dilated.size(); i++) {
		for (auto cit = m_dilated[i].m_contour.curves_begin(); cit != m_dilated[i].m_contour.curves_end(); ++cit) {
			auto x_monotone = *cit;
			ArrCSTraits::Curve_2 curve = toCurve(x_monotone);
			curves.emplace_back(curve);
			curves_data.emplace_back(curve, i);
		}
	}

	CGAL::insert(m_arr, curves.begin(), curves.end());
	// Set for each face which pattern it is a subset of.
	for (auto fit = m_arr.faces_begin(); fit != m_arr.faces_end(); ++fit) {
		if (fit->is_unbounded()) continue;
		auto pt = get_point_in(*fit);
		std::vector<int> origins;
		for (int i = 0; i < m_dilated.size(); i++) {
			if (onOrInside(m_dilated[i].m_contour, pt)) {
				origins.push_back(i);
				m_iToFaces[i].push_back(fit);
			}
		}
		fit->set_data(FaceData{origins});
	}

	// Store in each half-edges form which pattern it originates.
	for (auto cit = m_arr.curves_begin(); cit != m_arr.curves_end(); ++cit) {
		ArrCSTraits::Curve_2 curve = *cit;
		auto curve_data = std::find_if(curves_data.begin(), curves_data.end(), [&curve](const auto& pair) {
			auto other = pair.first;
			return curve.source() == other.source() && curve.target() == other.target() &&
					(curve.is_linear() && other.is_linear() ||
			         curve.is_circular() && other.is_circular() && curve.supporting_circle() == other.supporting_circle());
		});
		for (auto eit = m_arr.induced_edges_begin(cit); eit != m_arr.induced_edges_end(cit); ++eit) {
			DilatedPatternArrangement::Halfedge_handle eh = *eit;
			eh->data().origins.push_back(curve_data->second);
			eh->twin()->data().origins.push_back(curve_data->second);
		}
	}

	// Sort for std::set_intersection operation
	for (int i = 0; i < m_dilated.size(); i++) {
		auto& facesI = m_iToFaces[i];
		std::sort(facesI.begin(), facesI.end());
	}

	for (int i = 0; i < m_dilated.size(); i++) {
		for (int j = i + 1; j < m_dilated.size(); j++) {
			auto cs = intersectionComponents(i, j);
			for (auto& c : cs) {
				auto rel = computePreference(i, j, c);
				for (auto fit = c.faces_begin(); fit != c.faces_end(); ++fit) {
					fit->data().relations.push_back(rel);
				}
			}
		}
	}

	auto hEdges = hyperedges();
	for (auto edge : hEdges) {
		auto order = getRelationOrder(*edge);
		if (!order.has_value()) {
			std::cerr << "Hyperedge has cycle; ignoring preferences in this hyperedge." << std::endl;
			for (const auto& r : edge->relations) {
				r->ordering = Order::EQUAL;
			}
			order = getRelationOrder(*edge);
			assert(order.has_value());
		}
		setRelationOrder(*edge, *order);
	}

	for (auto fit = m_arr.faces_begin(); fit != m_arr.faces_end(); ++fit) {
		auto& data = fit->data();
		if (data.origins.empty()) continue;
		auto ordering = computeTotalOrder(data.origins, data.relations);
		if (!ordering.has_value()) {
			throw std::runtime_error("Impossible: no total order in a face");
		}
		data.ordering = *ordering;
	}

	for (int i = 0; i < m_dilated.size(); ++i) {
		auto cs = intersectionComponents(i);
		for (auto& c : cs) {
			std::unordered_set<int> avoidees;
			for (auto fit = c.faces_begin(); fit != c.faces_end(); ++fit) {
				for (int j : fit->data().ordering) {
					if (j == i) break;
					avoidees.insert(j);
				}
			}
			if (avoidees.empty())
				continue;

			auto bpis = boundaryParts(c, i);
			auto disks = includeExcludeDisks(i, avoidees, c);
			auto inclDisks = disks.include;
			auto exclDisks = disks.exclude;

			if (exclDisks.empty()) {
				continue;
			}
			auto componentPolygon = ccb_to_general_polygon<ArrCSTraits>(c.outer_ccb());
			auto morphedComponentPolygon = morph(bpis, componentPolygon, inclDisks, exclDisks, m_gs, m_cds);

			// Compute the morphed version of the CSPolygon for this component.
			// Set, for each face in component c, the morphed face to the intersection of this CSPolygon with the face.
			// Set the morphed edges to the intersection of the boundary of the polygon with the face.
			for (auto fit = c.faces_begin(); fit != c.faces_end(); ++fit) {
				auto facePolygon = face_to_polygon(*fit);

				for (const auto& bp : bpis) {
					CSPolyline mb = associatedBoundary(componentPolygon, morphedComponentPolygon, bp);
					intersection(mb, facePolygon, std::back_inserter(fit->data().morphedEdges[i]), false, true);
				}

				std::vector<CSPolygonWithHoles> morphedFacePolygonsWithHoles;

				CGAL::intersection(morphedComponentPolygon, facePolygon, std::back_inserter(morphedFacePolygonsWithHoles));
				auto& mf = fit->data().morphedFace[i];
				for (const auto& morphedFacePolygonWithHoles : morphedFacePolygonsWithHoles) {
					assert(!morphedFacePolygonWithHoles.has_holes()); // todo
					mf.push_back(morphedFacePolygonWithHoles.outer_boundary());
				}
			}
		}
	}
}

bool overlap(const Circle<Inexact>& c1, const Circle<Inexact>& c2) {
	return sqrt(CGAL::squared_distance(c1.center(), c2.center())) <= sqrt(c1.squared_radius()) + sqrt(c2.squared_radius());
}

std::vector<std::vector<std::pair<int, Circle<Inexact>>>> connectedDisks(const std::vector<Circle<Inexact>>& disks) {
	typedef std::vector<std::pair<int, Circle<Inexact>>> Component;
	std::vector<Component> components;

	auto merge = [&components](std::vector<Component*>& comps) {
		Component newComponent;
		for (auto& comp : comps) {
			std::copy(comp->begin(), comp->end(), std::back_inserter(newComponent));
			comp->clear();
		}
		components.erase(std::remove_if(components.begin(), components.end(), [](const Component& cs) {
			return cs.empty();
		}), components.end());
		return newComponent;
	};

	for (int i = 0; i < disks.size(); ++i) {
		const auto& d = disks[i];
		std::vector<Component*> overlaps;
		for (auto& comp : components) {
			bool anyOverlap = false;
			for (const auto& other : comp) {
				if (overlap(d, other.second)) {
					anyOverlap = true;
					break;
				}
			}
			if (anyOverlap) {
				overlaps.push_back(&comp);
			}
		}
		auto merged = merge(overlaps);
		merged.emplace_back(i, d);
		components.push_back(merged);
	}

	return components;
}

CSPolygon thinRectangle(const Point<Exact>& p, const OneRootPoint& n, const Number<Exact>& w) {
	Point<Inexact> nApproxInexact = approximateOneRootPoint(n);
	Point<Exact> nApprox(nApproxInexact.x(), nApproxInexact.y());
	Vector<Exact> d = nApprox - p;
	auto dl = sqrt(CGAL::to_double(d.squared_length()));
	Vector<Exact> normalized = d / dl;
	auto perp = normalized.perpendicular(CGAL::COUNTERCLOCKWISE) * w / 2;

	Point<Exact> p1 = p - perp;
	Point<Exact> p2 = nApprox + normalized * w / 10 - perp;
	Point<Exact> p3 = nApprox + normalized * w / 10 + perp;
	Point<Exact> p4 = p + perp;

	std::vector<CSCurve> curves({
                                        CSCurve(p1, p2),
                                        CSCurve(p2, p3),
                                        CSCurve(p3, p4),
                                        CSCurve(p4, p1)
	});

	std::vector<CSXMCurve> xm_curves;
	curvesToXMonotoneCurves(curves.begin(), curves.end(), std::back_inserter(xm_curves));

	return {xm_curves.begin(), xm_curves.end()};
}

CSPolygon morph(const std::vector<CSPolyline>& boundaryParts, const CSPolygon& componentShape, const std::vector<Circle<Exact>>& inclDisks,
				 const std::vector<Circle<Exact>>& exclDisks, const GeneralSettings& gs, const ComputeDrawingSettings& cds) {
	if (exclDisks.empty()) return componentShape;

	std::vector<Circle<Exact>> lineCovering;
	std::vector<Circle<Exact>> arcCovering;

	for (const auto& d : exclDisks) {
		std::vector<CSPolyline> inter;
		for (const auto& boundaryPart : boundaryParts) {
			intersection(boundaryPart, circleToCSPolygon(d), std::back_inserter(inter), false, true);
		}
		bool coversLine = true;
		for (const auto& p : inter) {
			if (!isStraight(p)) {
				coversLine = false;
				break;
			}
		}
		if (coversLine) {
			lineCovering.push_back(d);
		} else {
			arcCovering.push_back(d);
		}
	}

	auto dr = gs.dilationRadius();
	// Smoothing radius
	auto sr = cds.smoothingRadiusFactor * dr;

	std::vector<Circle<Inexact>> expandedLineCoveringDisks;
	for (const auto& d : lineCovering) {
		expandedLineCoveringDisks.emplace_back(approximate(d.center()), squared(sqrt(CGAL::to_double(d.squared_radius())) + CGAL::to_double(sr)));
	}
	auto diskComponents = connectedDisks(expandedLineCoveringDisks);

	auto nearestOnBoundary = [&boundaryParts](const Point<Exact>& point) -> std::pair<const CSPolyline*, OneRootPoint> {
		std::optional<OneRootNumber> minSqrdDist;
		std::optional<OneRootPoint> closestPoint;
	  	const CSPolyline* closestBp;
		for (const auto& bp : boundaryParts) {
			auto n = nearest(bp, point);
			auto sqrdDist = CGAL::square(n.x() - point.x()) + CGAL::square(n.y() - point.y());
			if (!minSqrdDist.has_value() || sqrdDist < *minSqrdDist) {
				minSqrdDist = sqrdDist;
				closestPoint = n;
				closestBp = &bp;
			}
		}
		return {closestBp, *closestPoint};
	};

	CSPolygonSet dilatedCuts;
	CSPolygonSet veryDilatedCuts;
	for (const auto& comp : diskComponents) {
		std::vector<Circle<Exact>> disks;
		for (const auto& [i, _] : comp) {
			disks.push_back(lineCovering[i]);
		}
		auto hull = approximateConvexHull(disks);
		CSPolygonSet cutSet(hull);

		// Only cut out a rectangle to the nearest disk of the component
		std::optional<Number<Exact>> minDist;
		std::optional<Circle<Exact>> closestDisk;
		std::optional<OneRootPoint> nearest;
		const CSPolyline* closestBp;

		for (const auto& d : disks) {
			auto [cbp, n] = nearestOnBoundary(d.center());
			auto dist = CGAL::squared_distance(approximateOneRootPoint(n), approximate(d.center()));
			if (!minDist.has_value() || dist < *minDist) {
				closestBp = cbp;
				if (inside(componentShape, d.center())) {
					minDist = dist;
					nearest = n;
					closestDisk = d;
				}
			}
		}

		if (closestDisk.has_value()) {
			cutSet.join(thinRectangle(closestDisk->center(), *nearest, gs.pointSize));
		}

		if (cds.smooth) {
			auto dilatedCutSet = approximateDilate(cutSet, 8 * sr);
			auto bp = *closestBp;
			auto [bpE, bpEstart, bpEtarget] = approximateExtend(bp, CGAL::to_double(8 * sr), dr);
			auto pgn = closeAroundBB(bpE, CGAL::COUNTERCLOCKWISE, 4 * CGAL::to_double(dr), bpEstart,
			                         bpEtarget);
			if (pgn.orientation() == CGAL::CLOCKWISE) {
				pgn.reverse_orientation();
			}
			dilatedCutSet.difference(pgn);

			dilatedCutSet.join(hull);
			if (closestDisk.has_value()) {
				dilatedCutSet.join(thinRectangle(closestDisk->center(), *nearest, gs.pointSize));
			}

			veryDilatedCuts.join(dilatedCutSet);
			auto drDilatedCutSet = approximateDilate(cutSet, 1.49 * sr);
			dilatedCuts.join(drDilatedCutSet);
		} else {
			std::vector<CSPolygonWithHoles> withHoles;
			cutSet.polygons_with_holes(std::back_inserter(withHoles));
			for (const auto& wh : withHoles) {
				veryDilatedCuts.join(wh);
			}
		}
	}

	for (const auto& d : arcCovering) {
		CSPolygonSet cutSet(circleToCSPolygon(d));

		auto [bp, n] = nearestOnBoundary(d.center());
		if (inside(componentShape, d.center())) {
			cutSet.join(thinRectangle(d.center(), n, gs.pointSize));
		}

		if (cds.smooth) {
			auto dilatedCutSet = approximateDilate(cutSet, 8 * sr);
			auto [bpE, bpEstart, bpEtarget] = approximateExtend(*bp, CGAL::to_double(8 * sr), dr);
			auto pgn = closeAroundBB(bpE, CGAL::COUNTERCLOCKWISE, 4 * CGAL::to_double(dr), bpEstart,
			                         bpEtarget);
			GpsCSTraits traits;
			if (pgn.orientation() == CGAL::CLOCKWISE) {
				pgn.reverse_orientation();
			}

			dilatedCutSet.difference(pgn);

			dilatedCutSet.join(circleToCSPolygon(d));
			if (inside(componentShape, d.center())) {
				dilatedCutSet.join(thinRectangle(d.center(), n, gs.pointSize));
			}

			veryDilatedCuts.join(dilatedCutSet);
			auto drDilatedCutSet = approximateDilate(cutSet, 1.49 * sr);
			dilatedCuts.join(drDilatedCutSet);
		} else {
			std::vector<CSPolygonWithHoles> withHoles;
			cutSet.polygons_with_holes(std::back_inserter(withHoles));
			for (const auto& wh : withHoles) {
				veryDilatedCuts.join(wh);
			}
		}
	}

	for (const auto& d: inclDisks) {
		veryDilatedCuts.difference(circleToCSPolygon(d));
	}

	CSPolygonSet smoothedSet;
	if (cds.smooth) {
		smoothedSet = approximateSmoothCO(veryDilatedCuts, sr);
		smoothedSet.intersection(dilatedCuts);
	} else {
		smoothedSet = veryDilatedCuts;
	}

	std::vector<CSPolygonWithHoles> modifiedCuts2;
	smoothedSet.polygons_with_holes(std::back_inserter(modifiedCuts2));

	CSPolygonSet polygonSet2(componentShape);
	for (const auto& modifiedCut : modifiedCuts2) {
		polygonSet2.difference(modifiedCut);
	}

	std::vector<CSPolygonWithHoles> polys;
	polygonSet2.polygons_with_holes(std::back_inserter(polys));
;
	CSPolygonWithHoles poly;
	std::optional<Number<Inexact>> max_area;
	for (const auto& cp : polys) {
		auto a = area(cp);
		if (!max_area.has_value() || a > *max_area) {
			max_area = a;
			poly = cp;
		}
	}

	// If poly has holes ignore them (that is, cut them out as well).
	auto outer = poly.outer_boundary();
	return outer;
}

CSPolyline associatedBoundary(const CSPolygon& component, const CSPolygon& morphedComponent, const CSPolyline& boundaryPart) {
	std::vector<CSXMCurve> morphed_xm_curves;
	for (auto cit = morphedComponent.curves_begin(); cit != morphedComponent.curves_end(); ++cit) {
		morphed_xm_curves.push_back(*cit);
	}
	auto boundaryPartStart = boundaryPart.curves_begin()->source();
	auto endIt = boundaryPart.curves_end();
	--endIt;
	auto boundaryPartEnd = endIt->target();
	int startIndex = -1;
	int endIndex = -1;

	for (int i = 0; i < morphed_xm_curves.size(); i++) {
		auto c = morphed_xm_curves[i];
		if (morphed_xm_curves[i].source() == boundaryPartStart) {
			startIndex = i;
		}
		if (morphed_xm_curves[i].target() == boundaryPartEnd) {
			endIndex = i;
		}
	}

	for (int i = 0; i < morphed_xm_curves.size() && (startIndex < 0 || endIndex < 0); i++) {
		const auto& c = morphed_xm_curves[i];
		if (startIndex < 0 && liesOn(c.source(), component).has_value() && !liesOn(c, component)) {
			startIndex = i;
		}
		if (endIndex < 0 && !liesOn(c, component) && liesOn(c.target(), component).has_value()) {
			endIndex = i;
		}
	}

	assert(startIndex >= 0);
	assert(endIndex >= 0);

	std::vector<CSXMCurve> xm_curves;

	if (endIndex >= startIndex) {
		for (int i = startIndex; i <= endIndex; ++i) {
			xm_curves.push_back(morphed_xm_curves[i]);
		}
	} else {
		for (int i = startIndex; i < morphed_xm_curves.size(); ++i) {
			xm_curves.push_back(morphed_xm_curves[i]);
		}
		for (int i = 0; i <= endIndex; ++i) {
			xm_curves.push_back(morphed_xm_curves[i]);
		}
	}

	return {xm_curves.begin(), xm_curves.end()};
}

/// Returns parts of the boundary of c that originate from i.
/// This function assumes that some part of the boundary, but not all of the boundary, originates from i.
std::vector<CSPolyline> boundaryParts(const CComponent& c, int i) {
	std::vector<CComponent::ComponentCcbCirculator> ccbs;
	std::copy(c.outer_ccbs_begin(), c.outer_ccbs_end(), std::back_inserter(ccbs));
	std::copy(c.inner_ccbs_begin(), c.inner_ccbs_end(), std::back_inserter(ccbs));

	std::vector<CSPolyline> polylines;

	for (const auto& ccb : ccbs) {
		boundaryParts<ArrCSTraits>(ccb, i, std::back_inserter(polylines));
	}

	return polylines;
}

/// Returns parts of the boundary of c that originate from i.
/// This function assumes that some part of the boundary, but not all of the boundary, originates from i.
std::vector<CSPolyline> boundaryParts(FaceH fh, int i) {
	std::vector<DilatedPatternArrangement::Ccb_halfedge_circulator> ccbs;
	std::copy(fh->outer_ccbs_begin(), fh->outer_ccbs_end(), std::back_inserter(ccbs));
	std::copy(fh->inner_ccbs_begin(), fh->inner_ccbs_end(), std::back_inserter(ccbs));

	std::vector<CSPolyline> polylines;

	for (const auto& ccb : ccbs) {
		boundaryParts<ArrCSTraits>(ccb, i, std::back_inserter(polylines));
	}

	return polylines;
}

bool isStraight(const CSPolyline& polyline) {
    return approximateAbsoluteTurningAngle(polyline) < 0.1;
}

/// The inclusion and exclusion disks for component \ref c when \ref i is stacked on top of \ref js.
IncludeExcludeDisks
DilatedPatternDrawing::includeExcludeDisks(int i, const std::unordered_set<int>& js, const CComponent& c) const {
	std::vector<Point<Exact>> ptsI;
	const auto& catPointsI = m_dilated[i].catPoints();
	std::transform(catPointsI.begin(), catPointsI.end(), std::back_inserter(ptsI), [](const CatPoint& catPoint) {
		return Point<Exact>(catPoint.point.x(), catPoint.point.y());
	});
	std::vector<Point<Exact>> ptsJs;
	for (int j : js) {
		const auto& catPointsJ = m_dilated[j].catPoints();
		std::transform(catPointsJ.begin(), catPointsJ.end(), std::back_inserter(ptsJs),
		               [](const CatPoint& catPoint) {
			               return Point<Exact>(catPoint.point.x(), catPoint.point.y());
		               });
	}

	auto rSqrd = m_gs.dilationRadius() * m_gs.dilationRadius();
    auto result = approximateGrowCircles(ptsI, ptsJs, rSqrd, rSqrd * m_cds.cutoutRadiusFactor * m_cds.cutoutRadiusFactor);

	std::vector<Circle<Exact>> relevantExclusionDisks;
	for (const auto& d : result.second) {
		if (CGAL::do_intersect(circleToCSPolygon(d), ccb_to_general_polygon<ArrCSTraits>(c.outer_ccb()))) {
			relevantExclusionDisks.push_back(d);
		}
	}
	return {result.first, relevantExclusionDisks};
}

/// The inclusion and exclusion disks for component \ref c when \ref i is stacked on top of \ref j.
IncludeExcludeDisks
DilatedPatternDrawing::includeExcludeDisks(int i, int j, const CComponent& c) const {
	std::unordered_set<int> js({j});
	return includeExcludeDisks(i, js, c);
}

std::shared_ptr<Relation> DilatedPatternDrawing::computePreference(int i, int j, const CComponent& c) {
	// The preference indicates the relation R in iRj.
	// If R is Order::GREATER then i > j and i is preferred to be on top of j.
	auto pref = Order::EQUAL;

	// 3. Prefer to cover a line segment over covering a circular arc.
	auto circArcIsCovered = [](const std::vector<CSPolyline>& bps) {
		for (auto& polyline : bps) {
			if (!isStraight(polyline)) {
				return true;
			}
		}
		return false;
	};

	// If j is stacked over i then it covers all edges of i.
	// Check if any of them is a circular arc.
	auto bpi = boundaryParts(c, i);
	assert(!bpi.empty());
	bool iCircArcIsCovered = circArcIsCovered(bpi);
	// Vice versa.
	auto bpj = boundaryParts(c, j);
	assert(!bpj.empty());
	bool jCircArcIsCovered = circArcIsCovered(bpj);

	if (iCircArcIsCovered && !jCircArcIsCovered) {
		pref = Order::GREATER;
	}
	if (!iCircArcIsCovered && jCircArcIsCovered){
		pref = Order::SMALLER;
	}

	// 2. Prefer to indent a line segment over indenting a circular arc.
	// Disks that would be cut out of i to expose points in j.
	auto jExclusion = includeExcludeDisks(i, j, c).exclude;
	// Disks that would be cut out of j to expose points in i.
	auto iExclusion = includeExcludeDisks(j, i, c).exclude;

	auto circularIndented = [](const std::vector<Circle<Exact>>& exclusionDisks, const std::vector<CSPolyline>& bps) {
		for (const auto& d : exclusionDisks) {
			if (d.squared_radius() <= 0) continue;
			for (auto& polyline : bps) {
				auto inters = intersection(polyline, circleToCSPolygon(d), true);
				for (auto& inter : inters) {
					if (!isStraight(inter)) {
						return true;
					}
				}
			}
		}
		return false;
	};
	bool iCircularIndented = circularIndented(jExclusion, bpi);
	bool jCircularIndented = circularIndented(iExclusion, bpj);

	if (iCircularIndented && !jCircularIndented) {
		pref = Order::SMALLER;
	}
	if (!iCircularIndented && jCircularIndented){
		pref = Order::GREATER;
	}

	// 1. Prefer to avoid few points over many points.
	// Fewer disks would be cut out of i than out of j, so prefer to stack i on top of j
	if (jExclusion.size() < iExclusion.size()) {
		pref = Order::GREATER;
	}
	if (iExclusion.size() < jExclusion.size()) {
		pref = Order::SMALLER;
	}

	return std::make_shared<Relation>(i, j, pref, pref);
}

Color whiten(const Color& color, double a) {
	return {static_cast<int>(255 * a + color.r * (1-a)), static_cast<int>(255 * a + color.g * (1-a)), static_cast<int>(255 * a + color.b * (1-a))};
}

void DilatedPatternDrawing::drawFaceFill(FaceH fh, renderer::GeometryRenderer& renderer,
                                     const GeneralSettings& gs, const DrawSettings& ds) const {
	auto& d = fh->data();
	for (int i : d.ordering) {
		renderer.setMode(GeometryRenderer::fill | GeometryRenderer::stroke);
		renderer.setFill(whiten(ds.getColor(m_dilated[i].category()), ds.whiten));
		renderer.setStroke(whiten(ds.getColor(m_dilated[i].category()), ds.whiten), ds.contourStrokeWeight(gs) / 1.5, true);
		if (!d.morphedFace.contains(i)) {
			auto poly = face_to_polygon(*fh);
			renderer.draw(renderPath(poly));
		} else {
			for (const auto& p : d.morphedFace[i]) {
				renderer.draw(renderPath(p));
			}
		}
	}
}

void DilatedPatternDrawing::drawFaceStroke(FaceH fh, renderer::GeometryRenderer& renderer,
										 const GeneralSettings& gs, const DrawSettings& ds) const {
	auto& d = fh->data();
	for (int index = 0; index < d.ordering.size(); ++index) {
		int i = d.ordering[index];

		std::vector<CSPolyline> polylines;
		if (d.morphedEdges[i].empty()) {
			auto bps = boundaryParts(fh, i);
			std::copy(bps.begin(), bps.end(), std::back_inserter(polylines));
		}
		std::copy(d.morphedEdges[i].begin(), d.morphedEdges[i].end(), std::back_inserter(polylines));


		std::vector<CSPolyline> modifiedPolylines;
		if (index == d.ordering.size() - 1) {
			modifiedPolylines = polylines;
		} else {
			bool nothingVisible = false;
			auto poly = face_to_polygon(*fh);
			auto bb = poly.bbox();
			Rectangle<Exact> bbX(bb.xmin() - 1, bb.ymin() - 1, bb.xmax() + 1, bb.ymax() + 1);
			std::vector<CSXMCurve> xm_cs;
			for (int k = 0; k < 4; ++k) {
				xm_cs.emplace_back(bbX.vertex(k), bbX.vertex((k + 1) % 4));
			}
			CSPolygon bbXPoly(xm_cs.begin(), xm_cs.end());

			CSPolygonSet polySet(bbXPoly);
			for (int higherIndex = index + 1; higherIndex < d.ordering.size(); ++higherIndex) {
				int j = d.ordering[higherIndex];
				// Pattern j is stacked on top of i and will cover the stroke of shape i.
				if (!d.morphedFace.contains(j)) {
					nothingVisible = true;
					break;
				} else {
					for (const auto& p : d.morphedFace[j]) {
						polySet.difference(p);
					}
				}
			}

			if (nothingVisible) continue;

			std::vector<CSPolygonWithHoles> polygonsWithHoles;
			polySet.polygons_with_holes(std::back_inserter(polygonsWithHoles));

			for (const auto& polyline : polylines) {
				for (const auto& polygon : polygonsWithHoles) {
					intersection(polyline, polygon, std::back_inserter(modifiedPolylines), false, false);
				}
			}
		}

		renderer.setMode(GeometryRenderer::stroke);
		renderer.setStroke(Color{0, 0, 0}, ds.contourStrokeWeight(gs), true);
		for (const auto& polyline : modifiedPolylines) {
			renderer.draw(renderPath(polyline));
		}
	}
}

std::optional<std::vector<int>> DilatedPatternDrawing::totalStackingOrder() const {
	std::vector<std::shared_ptr<Relation>> relations;
	std::vector<int> origins;
	for (int i = 0; i < m_dilated.size(); ++i) {
		origins.push_back(i);
		for (const auto& f : m_iToFaces.at(i)) {
			for (const auto& r : f->data().relations) {
				if (std::find_if(relations.begin(), relations.end(), [&r](const auto& it) { return it->left == r->left && it->right == r->right; }) == relations.end()) {
					relations.push_back(r);
				}
				assert(r->ordering != Order::EQUAL);
			}
		}
	}
	return computeTotalOrder(origins, relations);
}

std::vector<CComponent>
DilatedPatternDrawing::intersectionComponents(int i, int j) const {
    std::vector<CComponent> comps;
	connectedComponents(m_arr, std::back_inserter(comps), [i, j](FaceH fh) {
		const auto& origins = fh->data().origins;
		return std::find(origins.begin(), origins.end(), i) != origins.end() &&
		       std::find(origins.begin(), origins.end(), j) != origins.end();
	});
    return comps;
}

std::vector<CComponent>
DilatedPatternDrawing::intersectionComponents(int i) const {
    std::vector<CComponent> comps;
	connectedComponents(m_arr, std::back_inserter(comps), [i](FaceH fh) {
		const auto& origins = fh->data().origins;
		return origins.size() > 1 && std::find(origins.begin(), origins.end(), i) != origins.end();
	});
    return comps;
}

std::vector<std::shared_ptr<Hyperedge>> DilatedPatternDrawing::hyperedges() const {
	std::vector<FaceCH> interesting;

	for (auto fit = m_arr.faces_begin(); fit != m_arr.faces_end(); ++fit) {
		if (fit->data().origins.size() >= 2) {
			interesting.push_back(fit);
		}
	}

	std::sort(interesting.begin(), interesting.end(), [](const auto& fh1, const auto& fh2) {
		return fh1->data().origins.size() < fh2->data().origins.size();
	});

	std::vector<std::vector<std::shared_ptr<Hyperedge>>> hyperedgesGrouped;

	std::vector<std::shared_ptr<Hyperedge>> currentGroup;
	std::optional<int> lastSize;
	for (const auto& fh : interesting) {
		if (lastSize.has_value() && fh->data().origins.size() != *lastSize && !currentGroup.empty()) {
			hyperedgesGrouped.push_back(currentGroup);
			currentGroup.clear();
		}
		auto he = std::make_shared<Hyperedge>(fh->data().origins, fh->data().relations);
		for (auto& r : he->relations) {
			r->hyperedges.push_back(he);
		}
		currentGroup.push_back(he);
		lastSize = fh->data().origins.size();
	}
	if (!currentGroup.empty()) {
		hyperedgesGrouped.push_back(currentGroup);
	}

	std::vector<std::pair<int, std::shared_ptr<Hyperedge>>> trashCan;
	for (int i = 0; i < hyperedgesGrouped.size(); i++) {
		if (i + 1 >= hyperedgesGrouped.size()) break;
		const auto& group = hyperedgesGrouped[i];
		for (const auto& hyperedge : group) {
			for (const auto& larger : hyperedgesGrouped[i+1]) {
				bool fullyContained = true;
				for (const auto& r : hyperedge->relations) {
					if (std::find(larger->relations.begin(), larger->relations.end(), r) == larger->relations.end()) {
						fullyContained = false;
					}
				}
				if (fullyContained) {
					// store hyperedge for removal
					trashCan.emplace_back(i, hyperedge);
					break;
				}
			}
		}
	}

	for (const auto& [i, r] : trashCan) {
		auto& group = hyperedgesGrouped[i];
		group.erase(std::remove(group.begin(), group.end(), r), group.end());
	}

	std::vector<std::shared_ptr<Hyperedge>> hyperedges;
	for (const auto& group : hyperedgesGrouped) {
		std::copy(group.begin(), group.end(), std::back_inserter(hyperedges));
	}

	return hyperedges;
}

std::optional<std::vector<int>> computeTotalOrder(const std::vector<int>& origins, const std::vector<std::shared_ptr<Relation>>& relations) {
	if (relations.empty()) {
		return origins;
	}

	struct Vertex {
		int i;
		std::vector<Vertex*> neighbors;
		bool hasIncoming = false;
		int mark = 0;
	};

	std::unordered_map<int, Vertex> vertices;

	for (int i : origins) {
		vertices[i] = Vertex{i};
	}

	for (const auto& r : relations) {
		if (r->ordering == Order::EQUAL) continue;
		auto& u = vertices.at(r->left);
		auto& v = vertices.at(r->right);

		if (r->ordering == Order::SMALLER) {
			v.neighbors.push_back(&u);
			u.hasIncoming = true;
		} else {
			u.neighbors.push_back(&v);
			v.hasIncoming = true;
		}
	}

	std::vector<int> ordering;

	std::function<bool(Vertex&)> visit;

	visit = [&ordering, &visit](Vertex& u) {
		if (u.mark == 2) return true;
		if (u.mark == 1) return false;

		u.mark = 1;

		for (auto& v : u.neighbors) {
			bool success = visit(*v);
			if (!success) return false;
		}

		u.mark = 2;
		ordering.push_back(u.i);
		return true;
	};

	for (auto& [_, v] : vertices) {
		if (!v.hasIncoming) {
			bool success = visit(v);
			if (!success)
				return std::nullopt;
		}
	}

	if (ordering.size() != origins.size()) {
		return std::nullopt;
	}

	return ordering;
}

std::optional<std::vector<int>> getRelationOrder(const Hyperedge& e) {
	return computeTotalOrder(e.origins, e.relations);
}

void setRelationOrder(Hyperedge& e, const std::vector<int>& ordering) {
	for (auto& r : e.relations) {
		int i = std::find(ordering.begin(), ordering.end(), r->left) - ordering.begin();
		int j = std::find(ordering.begin(), ordering.end(), r->right) - ordering.begin();
		if (i < j) {
			r->ordering = Order::SMALLER;
		} else {
			r->ordering = Order::GREATER;
		}
	}
}

std::string to_string(Order ord) {
	if (ord == Order::SMALLER) {
		return "<";
	} else if (ord == Order::EQUAL) {
		return "=";
	} else {
		return ">";
	}
}

SimpleSetsPainting::SimpleSetsPainting(const DilatedPatternDrawing& dpd, const DrawSettings& ds)
    : m_ds(ds), m_dpd(dpd) {}

void SimpleSetsPainting::paint(renderer::GeometryRenderer& renderer) const {
	auto stackingOrder = m_dpd.totalStackingOrder();
	// If there is a stacking order, draw the complete patterns stacked in that order
	if (stackingOrder.has_value()) {
		for (int i : *stackingOrder) {
            std::vector<CComponent> comps;
			connectedComponents(m_dpd.m_arr, std::back_inserter(comps), [i](const FaceH& fh) {
				const auto& ors = fh->data().origins;
				return std::find(ors.begin(), ors.end(), i) != ors.end();
			});
			assert(comps.size() == 1);
			const auto& comp = comps[0];

			std::vector<CSPolyline> boundaryPieces;
			for (auto fit = comp.faces_begin(); fit != comp.faces_end(); ++fit) {
				auto& data = fit->data();
				if (data.morphedFace.contains(i)) {
					std::copy(data.morphedEdges[i].begin(), data.morphedEdges[i].end(), std::back_inserter(boundaryPieces));
				} else {
					FaceH fh = fit.handle();
					auto bps = boundaryParts(fh, i);
					std::copy(bps.begin(), bps.end(), std::back_inserter(boundaryPieces));
				}
			}

			// no order or hash on OneRootPoint :(
//			std::map<OneRootPoint, int> sourceToI;
//			for (int j = 0; j < boundaryPieces.size(); ++j) {
//				auto& bp = boundaryPieces[j];
//				sourceToI[bp.curves_begin()->source()] = j;
//			}
			std::vector<CSXMCurve> xm_curves;

			std::copy(boundaryPieces[0].curves_begin(), boundaryPieces[0].curves_end(), std::back_inserter(xm_curves));
			int count = 1;
			while (count < boundaryPieces.size()) {
				auto head = xm_curves.back().target();
				for (const auto& bp : boundaryPieces) {
					if (bp.curves_begin()->source() == head) {
						std::copy(bp.curves_begin(), bp.curves_end(), std::back_inserter(xm_curves));
					}
				}
				++count;
			}

			CSPolygon csPolygon(xm_curves.begin(), xm_curves.end());

			renderer.setMode(GeometryRenderer::fill | GeometryRenderer::stroke);
			renderer.setFill(whiten(m_ds.getColor(m_dpd.m_dilated[i].category()), m_ds.whiten));
			renderer.setStroke(Color{0, 0, 0}, m_ds.contourStrokeWeight(m_dpd.m_gs), true);
			renderer.draw(renderPath(csPolygon));
		}
	} else {
		// If there is no stacking order, draw each face of the arrangement separately
		for (auto fit = m_dpd.m_arr.faces_begin(); fit != m_dpd.m_arr.faces_end(); ++fit) {
			if (fit->is_unbounded())
				continue;
			m_dpd.drawFaceFill(fit.ptr(), renderer, m_dpd.m_gs, m_ds);
		}
		for (auto fit = m_dpd.m_arr.faces_begin(); fit != m_dpd.m_arr.faces_end(); ++fit) {
			if (fit->is_unbounded())
				continue;
			m_dpd.drawFaceStroke(fit.ptr(), renderer, m_dpd.m_gs, m_ds);
		}
	}

	// Draw points
	const auto& gs = m_dpd.m_gs;
	renderer.setStroke(Color{0, 0, 0}, m_ds.pointStrokeWeight(gs), true);
	renderer.setFillOpacity(255);
	renderer.setMode(renderer::GeometryRenderer::fill | renderer::GeometryRenderer::stroke);
	for (const auto& dp : m_dpd.m_dilated) {
		for (const auto& cp : dp.catPoints()) {
			renderer.setFill(m_ds.getColor(cp.category));
			renderer.draw(Circle<Inexact>{cp.point, gs.pointSize * gs.pointSize});
		}
	}
}
}