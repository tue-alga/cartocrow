#include "drawing_algorithm.h"
#include "dilated/dilated_poly.h"
#include "../renderer/render_path.h"

using namespace cartocrow::renderer;

namespace cartocrow::simplesets {
CSTraits::X_monotone_curve_2 reverse(CSTraits::X_monotone_curve_2 curve) {
	if (curve.is_linear()) {
		return CSTraits::X_monotone_curve_2(curve.supporting_line(), curve.target(), curve.source());
	} else {
		return CSTraits::X_monotone_curve_2(curve.supporting_circle(), curve.target(), curve.source(), -curve.orientation());
	}
}

CSPolygon face_to_polygon(const DilatedPatternArrangement::Face& face) {
	assert(face.number_of_holes() == 0);
	auto ccb_start = face.outer_ccb();
	auto ccb = ccb_start;

	std::vector<CSTraits::X_monotone_curve_2> x_monotone_curves;
	do {
		auto curve = ccb->curve();
		if (ccb->source()->point() == curve.source()) {
			x_monotone_curves.push_back(curve);
		} else {
			x_monotone_curves.push_back(reverse(curve));
		}
	} while(++ccb != ccb_start);

	return {x_monotone_curves.begin(), x_monotone_curves.end()};
}

CSTraits::X_monotone_curve_2 make_x_monotone(const Segment<Exact>& segment) {
	CSTraits traits;
	auto make_x_monotone = traits.make_x_monotone_2_object();
	std::vector<boost::variant<CSTraits::Point_2, CSTraits::X_monotone_curve_2>> curves_and_points;
	make_x_monotone(segment, std::back_inserter(curves_and_points));
	std::vector<CSTraits::X_monotone_curve_2> curves;

	// There should not be any isolated points
	for (auto kinda_curve : curves_and_points) {
		assert(kinda_curve.which() == 1);
		auto curve = boost::get<CSTraits::X_monotone_curve_2>(kinda_curve);
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
		std::vector<boost::variant<CSTraits::Point_2, CSTraits::X_monotone_curve_2>> curves_and_points;
//		make_x_monotone(circle, std::back_inserter(curves_and_points));
		make_x_monotone(seg, std::back_inserter(curves_and_points));
		std::vector<CSTraits::X_monotone_curve_2> curves;

		// There should not be any isolated points
		for (auto kinda_curve : curves_and_points) {
			auto curve = boost::get<CSTraits::X_monotone_curve_2>(kinda_curve);
			curves.push_back(curve);
		}

		typedef std::pair<CSTraits::Point_2, unsigned int> Intersection_point;
		typedef boost::variant<Intersection_point, CSTraits::X_monotone_curve_2> Intersection_result;
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
		typedef boost::variant<Intersection_point, CSTraits::X_monotone_curve_2> Intersection_result;
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
	typedef boost::variant<Intersection_point, CSTraits::X_monotone_curve_2> Intersection_result;
	std::vector<Intersection_result> intersection_results;

	for (auto cit = polygon.curves_begin(); cit != polygon.curves_end(); ++cit) {
		const auto& curve = *cit;
		curve.intersect(seg_curve, std::back_inserter(intersection_results));
	}

	return intersection_results.size() % 2 == 1;
}

std::vector<Component>
connectedComponents(const std::vector<FaceH>& faces) {
	std::vector<FaceH> remaining = faces;
	std::vector<Component> components;

	while (!remaining.empty()) {
		// We do a BFS
		std::vector<FaceH> compFaces;
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

					// If this is one of the provided faces, and not yet added to queue or compFaces, add it to queue.
					if (std::find(remaining.begin(), remaining.end(), candidate) != remaining.end() &&
					    std::find(compFaces.begin(), compFaces.end(), candidate) == compFaces.end() &&
					    std::find(q.begin(), q.end(), candidate) == q.end()) {
						q.push_back(candidate);
					}
				} while (++ccb_it != ccb_start);
			}
		}

		// Done with this connected component
		remaining.erase(std::remove_if(remaining.begin(), remaining.end(), [&compFaces](const auto& f) {
		  return std::find(compFaces.begin(), compFaces.end(), f) != compFaces.end();
		}), remaining.end());
		components.emplace_back(std::move(compFaces));
	}

	return components;
}

// This observer only works as intended when inserting curves incrementally, not when using the CGAL sweep-line insert.
class MyObserver : public CGAL::Arr_observer<DilatedPatternArrangement> {
  public:
	MyObserver(DilatedPatternArrangement& arr,
	           const std::vector<std::pair<CSTraits::X_monotone_curve_2, int>>& curve_data) :
	      CGAL::Arr_observer<DilatedPatternArrangement>(arr), m_curve_data(curve_data) {};

	void after_create_edge(HalfEdgeH e) override {
		const CSTraits::X_monotone_curve_2& curve = e->curve();
		bool found = false;
		for (const auto& curve_datum : m_curve_data) {
			const CSTraits::X_monotone_curve_2& other_curve = curve_datum.first;
			// Cannot check curve equality for some reason, so do it manually instead.
			if ((curve.source() == other_curve.source() &&
			    curve.target() == other_curve.target() ||
			    curve.source() == other_curve.target() &&
			     curve.target() == other_curve.source()) &&
			    (curve.is_linear() && other_curve.is_linear() ||
			     curve.is_circular() && other_curve.is_circular() &&
			         curve.supporting_circle() == other_curve.supporting_circle())) {
				found = true;
				e->set_data(HalfEdgeData{curve_datum.second});
			}
		}

		++m_count;
	}

	void before_split_edge(HalfEdgeH e, VertexH v, const X_monotone_curve_2& c1, const X_monotone_curve_2& c2) override {
		m_edge_split_data = e->data();
	}

	void after_split_edge(HalfEdgeH e1, HalfEdgeH e2) override {
		if (!m_edge_split_data.has_value()) {
			throw std::runtime_error("Data before split is not accessible after split");
		}
		e1->set_data(*m_edge_split_data);
		e2->set_data(*m_edge_split_data);

		m_edge_split_data = std::nullopt;
	}

	int m_count = 0;
  private:
	const std::vector<std::pair<CSTraits::X_monotone_curve_2, int>>& m_curve_data;
	std::optional<HalfEdgeData> m_edge_split_data;
};

CSTraits::Curve_2 to_curve(CSTraits::X_monotone_curve_2 xmc) {
	if (xmc.is_linear()) {
		return {xmc.supporting_line(), xmc.source(), xmc.target()};
	} else if (xmc.is_circular()) {
		return {xmc.supporting_circle(), xmc.source(), xmc.target()};
	} else {
		throw std::runtime_error("Impossible: circle-segment x-monotone curve is neither linear nor circular.");
	}
}

//DilatedPatternArrangement
//dilateAndArrange(const Partition& partition, const GeneralSettings& gs, const ComputeDrawingSettings& cds) {
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

//	MyObserver obs(m_arr, curves_data);
//	for (auto& curve : x_monotone_curves) {
//		CGAL::insert(m_arr, curve);
//	}

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
				for (auto f : c.m_faces) {
					f->data().relations.push_back(rel);
				}
			}
		}
	}
}

Relation DilatedPatternDrawing::computePreference(int i, int j, const Component& c) {
	auto pref = CGAL::ZERO;

	// 3. Prefer to cover a line segment over covering a circular arc.
	// 2. Prefer to indent a line segment over indenting a circular arc.
	// 1. Prefer to avoid few points over many points.
	// todo: implement

	return {i, j, pref, pref};
}

std::vector<Component>
DilatedPatternDrawing::intersectionComponents(int i, int j) {
	std::vector<FaceH> faces;
	const std::vector<FaceH>& facesI = m_iToFaces[i];
	const std::vector<FaceH>& facesJ = m_iToFaces[j];
	std::set_intersection(facesI.begin(), facesI.end(), facesJ.begin(), facesJ.end(), std::back_inserter(faces));
	return connectedComponents(faces);
}

std::vector<Component>
DilatedPatternDrawing::intersectionComponents(int i) {
	std::vector<FaceH> faces;
	std::copy_if(m_iToFaces[i].begin(), m_iToFaces[i].end(), std::back_inserter(faces),
	             [](const FaceH& fh) { return fh->data().origins.size() > 1; });
	return connectedComponents(faces);
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