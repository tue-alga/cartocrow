#include "drawing_algorithm.h"

#include <utility>
#include "dilated/dilated_poly.h"
#include "../renderer/render_path.h"
#include "grow_circles.h"
#include "poly_line_gon_intersection.h"
#include "helpers/arrangement_helpers.h"
#include <CGAL/Boolean_set_operations_2.h>

using namespace cartocrow::renderer;

namespace cartocrow::simplesets {
// todo: deal with holes
CSPolygon face_to_polygon(const DilatedPatternArrangement::Face& face) {
	assert(face.number_of_holes() == 0);
	return ccb_to_polygon<CSTraits>(face.outer_ccb());
}

X_monotone_curve_2 make_x_monotone(const Segment<Exact>& segment) {
	CSTraits traits;
	auto make_x_monotone = traits.make_x_monotone_2_object();
	std::vector<boost::variant<CSTraits::Point_2, X_monotone_curve_2>> curves_and_points;
	make_x_monotone(segment, std::back_inserter(curves_and_points));
	std::vector<X_monotone_curve_2> curves;

	// There should not be any isolated points
	for (auto kinda_curve : curves_and_points) {
		assert(kinda_curve.which() == 1);
		auto curve = boost::get<X_monotone_curve_2>(kinda_curve);
		curves.push_back(curve);
	}

	assert(curves.size() == 1);
	return curves[0];
}

Point<Exact> get_approx_point_on_boundary(const DilatedPatternArrangement::Face& face) {
	auto curve = face.outer_ccb()->curve();
	Rectangle<Exact> bbox = curve.bbox();
	Rectangle<Exact> rect({bbox.xmin() - 1, bbox.ymin() - 1}, {bbox.xmax() + 1, bbox.ymax() + 1});
	Point<Exact> approx_source = makeExact(approximateAlgebraic(curve.source()));
	Point<Exact> approx_target = makeExact(approximateAlgebraic(curve.target()));
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
		CSTraits traits;
		auto make_x_monotone = traits.make_x_monotone_2_object();
		std::vector<boost::variant<CSTraits::Point_2, X_monotone_curve_2>> curves_and_points;
//		make_x_monotone(circle, std::back_inserter(curves_and_points));
		make_x_monotone(seg, std::back_inserter(curves_and_points));
		std::vector<X_monotone_curve_2> curves;

		// There should not be any isolated points
		for (auto kinda_curve : curves_and_points) {
			auto curve = boost::get<X_monotone_curve_2>(kinda_curve);
			curves.push_back(curve);
		}

		typedef std::pair<CSTraits::Point_2, unsigned int> Intersection_point;
		typedef boost::variant<Intersection_point, X_monotone_curve_2> Intersection_result;
		std::vector<Intersection_result> intersection_results;
		assert(curves.size() == 1);
		curve.intersect(curves[0], std::back_inserter(intersection_results));

		std::vector<CSTraits::Point_2> intersection_pts;
		for (const auto& result : intersection_results) {
			assert(result.which() == 0);
			intersection_pts.push_back(boost::get<Intersection_point>(result).first);
		}

		assert(intersection_pts.size() == 1);
		return makeExact(approximateAlgebraic(intersection_pts[0]));
	}
}

Point<Exact> get_point_in(const DilatedPatternArrangement::Face& face) {
	auto poly = face_to_polygon(face);
	Rectangle<Exact> bbox = poly.bbox();
	Rectangle<Exact> rect({bbox.xmin() - 1, bbox.ymin() - 1}, {bbox.xmax() + 1, bbox.ymax() + 1});
	Point<Exact> point_outside(rect.xmin(), rect.ymin());
	Point<Exact> approx_point_on_boundary = get_approx_point_on_boundary(face);
	Line<Exact> line(point_outside, approx_point_on_boundary);
	auto line_inter_box = CGAL::intersection(rect, line);
	auto seg = boost::get<Segment<Exact>>(*line_inter_box);
	auto seg_curve = make_x_monotone(seg);
	std::vector<CSTraits::Point_2> intersection_pts;
	for (auto cit = poly.curves_begin(); cit != poly.curves_end(); cit++) {
		auto curve = *cit;
		typedef std::pair<CSTraits::Point_2, unsigned int> Intersection_point;
		typedef boost::variant<Intersection_point, X_monotone_curve_2> Intersection_result;
		std::vector<Intersection_result> intersection_results;
		curve.intersect(seg_curve, std::back_inserter(intersection_results));

		for (const auto& result : intersection_results) {
			assert(result.which() == 0);
			intersection_pts.push_back(boost::get<Intersection_point>(result).first);
		}
	}

	std::sort(intersection_pts.begin(), intersection_pts.end(), [&seg](const auto& pt1, const auto& pt2) {
		Vector<Exact> v = seg.supporting_line().to_vector();
		return v * (makeExact(approximateAlgebraic(pt1)) - makeExact(approximateAlgebraic(pt2))) < 0;
	});

	Point<Exact> approx_source;
	Point<Exact> approx_target;
	if (intersection_pts.size() >= 2) {
		approx_source = makeExact(approximateAlgebraic(intersection_pts[0]));
		approx_target = makeExact(approximateAlgebraic(intersection_pts[1]));
	} else {
		approx_source = makeExact(approximateAlgebraic(intersection_pts[0]));
		approx_target = approx_point_on_boundary;
	}

	return midpoint(Segment<Exact>(approx_source, approx_target));
}

bool on_or_inside(const CSPolygon& polygon, const Point<Exact>& point) {
	Ray<Exact> ray(point, Vector<Exact>(1, 0));

	Rectangle<Exact> bbox = polygon.bbox();
	Rectangle<Exact> rect({bbox.xmin() - 1, bbox.ymin() - 1}, {bbox.xmax() + 1, bbox.ymax() + 1});

	auto inter = CGAL::intersection(ray, rect);
	if (!inter.has_value()) return false;
	auto seg = boost::get<Segment<Exact>>(*inter);
	auto seg_curve = make_x_monotone(seg);

	typedef std::pair<CSTraits::Point_2, unsigned int> Intersection_point;
	typedef boost::variant<Intersection_point, X_monotone_curve_2> Intersection_result;
	std::vector<Intersection_result> intersection_results;

	for (auto cit = polygon.curves_begin(); cit != polygon.curves_end(); ++cit) {
		const auto& curve = *cit;
		curve.intersect(seg_curve, std::back_inserter(intersection_results));
	}

	return intersection_results.size() % 2 == 1;
}

std::vector<Component>
connectedComponents(const DilatedPatternArrangement& arr, std::function<bool(FaceH)> in_component) {
	std::vector<FaceH> remaining;
	for (auto fit = arr.faces_begin(); fit != arr.faces_end(); ++fit) {
		auto fh = fit.ptr();
		if (in_component(fh)) {
			remaining.emplace_back(fh);
		}
	}

	std::vector<Component> components;

	while (!remaining.empty()) {
		// We do a BFS
		std::vector<FaceH> compFaces;
		std::vector<HalfEdgeH> compBoundaryEdges;
		auto first = remaining.front();
		std::deque<FaceH> q;
		q.push_back(first);

		while (!q.empty()) {
			auto f = q.front();
			q.pop_front();
			compFaces.push_back(f);

			// Go through boundaries of this face
			std::vector<DilatedPatternArrangement::Ccb_halfedge_circulator> ccbs;
			std::copy(f->outer_ccbs_begin(), f->outer_ccbs_end(), std::back_inserter(ccbs));
			std::copy(f->inner_ccbs_begin(), f->inner_ccbs_end(), std::back_inserter(ccbs));
			for (auto ccb_start : ccbs) {
				auto ccb_it = ccb_start;

				// Go through each neighbouring face
				do {
					auto candidate = ccb_it->twin()->face();
					if (!in_component(candidate)) {
						compBoundaryEdges.emplace_back(ccb_it.ptr());
					} else {
						// If this is one of the provided faces, and not yet added to queue or compFaces, add it to queue.
						if (std::find(compFaces.begin(), compFaces.end(), candidate) ==
						        compFaces.end() &&
						    std::find(q.begin(), q.end(), candidate) == q.end()) {
							q.push_back(candidate);
						}
					}
				} while (++ccb_it != ccb_start);
			}
		}

		// Done with this connected component
		remaining.erase(std::remove_if(remaining.begin(), remaining.end(), [&compFaces](const auto& f) {
		  return std::find(compFaces.begin(), compFaces.end(), f) != compFaces.end();
		}), remaining.end());
		components.emplace_back(std::move(compFaces), std::move(compBoundaryEdges), in_component);
	}

	return components;
}

CSTraits::Curve_2 to_curve(const X_monotone_curve_2& xmc) {
	if (xmc.is_linear()) {
		return {xmc.supporting_line(), xmc.source(), xmc.target()};
	} else if (xmc.is_circular()) {
		return {xmc.supporting_circle(), xmc.source(), xmc.target()};
	} else {
		throw std::runtime_error("Impossible: circle-segment x-monotone curve is neither linear nor circular.");
	}
}

Component::Component(std::vector<FaceH> faces, std::vector<HalfEdgeH> boundary_edges, std::function<bool(FaceH)> in_component) :
      m_faces(std::move(faces)), m_in_component(std::move(in_component)) {
	while (!boundary_edges.empty()) {
		auto he = boundary_edges.front();
		auto circ_start = ComponentCcbCirculator(he, m_in_component);
		auto circ = circ_start;

		std::vector<X_monotone_curve_2> xm_curves;
		auto last_it = boundary_edges.end();
		do {
			last_it = std::remove(boundary_edges.begin(), last_it, circ.handle());
			xm_curves.push_back(circ->curve());
		} while (++circ != circ_start);
		boundary_edges.erase(last_it, boundary_edges.end());

		CSPolygon polygon(xm_curves.begin(), xm_curves.end());
		auto orientation = polygon.orientation();
		if (orientation == CGAL::COUNTERCLOCKWISE) {
			m_outer_ccbs.push_back(circ_start);
		} else if (orientation == CGAL::CLOCKWISE) {
			m_inner_ccbs.push_back(circ_start);
		} else {
			throw std::runtime_error("Face orientation is not clockwise nor counterclockwise.");
		}
	}
}

std::ostream& operator<<(std::ostream& out, const Order& o) {
	return out << to_string(o);
}

std::ostream& operator<<(std::ostream& out, const Relation& r) {
	return out << r.left << " " << r.ordering
	           << (r.preference != r.ordering ? "(" + to_string(r.preference) + ")" : "")
	           << " " << r.right;
}

DilatedPatternDrawing::DilatedPatternDrawing(const Partition& partition, const GeneralSettings& gs, const ComputeDrawingSettings& cds)
	: m_gs(gs), m_cds(cds) {
	for (const auto& p : partition) {
		m_dilated.emplace_back(*p, gs.dilationRadius());
	}

	std::vector<CSTraits::Curve_2> curves;
	std::vector<std::pair<CSTraits::Curve_2, int>> curves_data;
	for (int i = 0; i < m_dilated.size(); i++) {
		for (auto cit = m_dilated[i].m_contour.curves_begin(); cit != m_dilated[i].m_contour.curves_end(); ++cit) {
			auto x_monotone = *cit;
			CSTraits::Curve_2 curve = to_curve(x_monotone);
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
			if (on_or_inside(m_dilated[i].m_contour, pt)) {
				origins.push_back(i);
				m_iToFaces[i].push_back(fit);
			}
		}
		fit->set_data(FaceData{origins});
	}

	// Store in each half-edges form which pattern it originates.
	for (auto cit = m_arr.curves_begin(); cit != m_arr.curves_end(); ++cit) {
		CSTraits::Curve_2 curve = *cit;
		auto curve_data = std::find_if(curves_data.begin(), curves_data.end(), [&curve](const auto& pair) {
			auto other = pair.first;
			return curve.source() == other.source() && curve.target() == other.target() &&
					(curve.is_linear() && other.is_linear() ||
			         curve.is_circular() && other.is_circular() && curve.supporting_circle() == other.supporting_circle());
		});
		for (auto eit = m_arr.induced_edges_begin(cit); eit != m_arr.induced_edges_end(cit); ++eit) {
			DilatedPatternArrangement::Halfedge_handle eh = *eit;
			HalfEdgeData data(curve_data->second);
			eh->set_data(data);
			eh->twin()->set_data(data);
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
}

/// Returns parts of the boundary of c that originate from i.
/// This function assumes that some part of the boundary, but not all of the boundary, originates from i.
std::vector<CSPolyline> boundaryParts(const Component& c, int i) {
	std::vector<Component::ComponentCcbCirculator> ccbs;
	std::copy(c.outer_ccbs_begin(), c.outer_ccbs_end(), std::back_inserter(ccbs));
	std::copy(c.inner_ccbs_begin(), c.inner_ccbs_end(), std::back_inserter(ccbs));

	std::vector<CSPolyline> polylines;

	for (const auto& ccb : ccbs) {
		// First find a half-edge at the start of a 'part' of this CCB (connected component of the boundary).
		auto circ = ccb;

		bool found = true;
		while (circ->data().origin != i) {
			++circ;
			if (circ == ccb) {
				found = false;
				break;
			}
		}

		if (!found) {
			continue;
		}

		while (true) {
			auto prev = circ;
			--prev;
			if (prev->data().origin == i) {
				circ = prev;
			} else {
				break;
			}
		}
		assert(circ->data().origin == i);

		// Next, make a polyline for every connected part of the boundary that originates from i.
		std::vector<X_monotone_curve_2> xm_curves;
		auto curr = circ;
		do {
			if (curr->data().origin == i) {
				xm_curves.push_back(curr->curve());
			} else {
				if (!xm_curves.empty()) {
					polylines.emplace_back(xm_curves.begin(), xm_curves.end());
					xm_curves.clear();
				}
			}
		} while (++curr != circ);
		if (!xm_curves.empty()) {
			polylines.emplace_back(xm_curves.begin(), xm_curves.end());
			xm_curves.clear();
		}
	}

	return polylines;
}

// todo: check if small circular arcs should be allowed
bool isStraight(const CSPolyline& polyline) {
	for (auto cit = polyline.curves_begin(); cit != polyline.curves_end(); ++cit) {
		if (cit->is_circular()) return false;
	}
	return true;
}

/// The inclusion and exclusion disks for component \ref c when \ref i is stacked on top of \ref j.
IncludeExcludeDisks
DilatedPatternDrawing::includeExcludeDisks(int i, int j, const Component& c) {
	std::vector<Point<Exact>> ptsI;
	const auto& catPointsI = m_dilated[i].catPoints();
	std::transform(catPointsI.begin(), catPointsI.end(), std::back_inserter(ptsI), [](const CatPoint& catPoint) {
		return Point<Exact>(catPoint.point.x(), catPoint.point.y());
	});
	std::vector<Point<Exact>> ptsJ;
	const auto& catPointsJ = m_dilated[j].catPoints();
	std::transform(catPointsJ.begin(), catPointsJ.end(), std::back_inserter(ptsJ), [](const CatPoint& catPoint) {
	  return Point<Exact>(catPoint.point.x(), catPoint.point.y());
	});

	auto rSqrd = m_gs.dilationRadius() * m_gs.dilationRadius();
    auto result = growCircles(ptsI, ptsJ, rSqrd, rSqrd * m_cds.cutoutRadiusFactor);

	std::vector<Circle<Exact>> relevantExclusionDisks;
	for (const auto& d : result.second) {
		if (CGAL::do_intersect(circleToCSPolygon(d), ccb_to_polygon<CSTraits>(c.outer_ccb()))) {
			relevantExclusionDisks.push_back(d);
		}
	}
	return {result.first, relevantExclusionDisks};
}

Relation DilatedPatternDrawing::computePreference(int i, int j, const Component& c) {
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
				auto inters = poly_line_gon_intersection(circleToCSPolygon(d), polyline);
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
		pref = Order::GREATER;
	}
	if (!iCircularIndented && jCircularIndented){
		pref = Order::SMALLER;
	}

	// 1. Prefer to avoid few points over many points.
	// Fewer disks would be cut out of i than out of j, so prefer to stack i on top of j
	if (jExclusion.size() < iExclusion.size()) {
		pref = Order::GREATER;
	}
	if (iExclusion.size() < jExclusion.size()) {
		pref = Order::SMALLER;
	}

	return {i, j, pref, pref};
}

std::vector<Component>
DilatedPatternDrawing::intersectionComponents(int i, int j) const {
	return connectedComponents(m_arr, [i, j](FaceH fh) {
		const auto& origins = fh->data().origins;
		return std::find(origins.begin(), origins.end(), i) != origins.end() &&
		       std::find(origins.begin(), origins.end(), j) != origins.end();
	});
}

std::vector<Component>
DilatedPatternDrawing::intersectionComponents(int i) const {
	return connectedComponents(m_arr, [i](FaceH fh) {
		const auto& origins = fh->data().origins;
		return std::find(origins.begin(), origins.end(), i) != origins.end();
	});
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
	renderer.setMode(renderer::GeometryRenderer::fill);
	for (auto fit = m_dpd.m_arr.faces_begin(); fit != m_dpd.m_arr.faces_end(); ++fit) {
		if (fit->is_unbounded()) continue;
		auto face = *fit;
		auto origins = face.data().origins;
		if (origins.size() > 1) {
			renderer.setFill(Color{100, 100, 100});
		} else if (origins.size() == 1) {
			renderer.setFill(m_ds.colors[m_dpd.m_dilated[origins[0]].category()]);
		} else {
			continue;
		}
		auto poly = face_to_polygon(face);

		RenderPath path;
		bool first = true;
		for (auto cit = poly.curves_begin(); cit != poly.curves_end(); ++cit) {
			if (first) {
				path.moveTo(approximateAlgebraic(cit->source()));
				first = false;
			}
			if (cit->is_linear()) {
				path.lineTo(approximateAlgebraic(cit->target()));
			} else if (cit->is_circular()){
				auto circle = cit->supporting_circle();
				path.arcTo(approximate(circle.center()), cit->orientation() == CGAL::CLOCKWISE, approximateAlgebraic(cit->target()));
			}
		}
		path.close();
		renderer.setFillOpacity(150);
		renderer.setMode(renderer::GeometryRenderer::fill);
		renderer.draw(path);

		renderer.setMode(renderer::GeometryRenderer::fill | renderer::GeometryRenderer::stroke);
		renderer.setFill(Color{0, 0, 0});
		renderer.setStroke(Color{0, 0, 0}, 1);

		auto& rs = face.data().relations;
		if (!rs.empty()) {
			auto r = rs[0];
			std::stringstream ss;
			ss << r.left << " " << to_string(r.preference) << " " << r.right;
			renderer.drawText(get_point_in(face), ss.str());
		}

		if (origins.size() == 1) {
			renderer.drawText(get_point_in(face), std::to_string(origins[0]));
		}
	}

	renderer.setMode(renderer::GeometryRenderer::stroke);
	for (auto eit = m_dpd.m_arr.edges_begin(); eit != m_dpd.m_arr.edges_end(); ++eit) {
		auto curve = eit->curve();
		renderer.setStroke(m_ds.colors[m_dpd.m_dilated[eit->data().origin].category()], m_ds.contourStrokeWeight(m_dpd.m_gs), true);
		if (curve.is_linear()) {
			renderer.draw(Segment<Inexact>(approximateAlgebraic(curve.source()), approximateAlgebraic(curve.target())));
		} else if (curve.is_circular()){
			RenderPath path;
			path.moveTo(approximateAlgebraic(curve.source()));
			auto circle = curve.supporting_circle();
			path.arcTo(approximate(circle.center()), curve.orientation() == CGAL::CLOCKWISE, approximateAlgebraic(curve.target()));
			renderer.draw(path);
		}
	}
}
}