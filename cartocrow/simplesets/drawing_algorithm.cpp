#include "drawing_algorithm.h"
#include "dilated/dilated_poly.h"

namespace cartocrow::simplesets {
CSPolygon face_to_polygon(const DilatedPatternArrangement::Face& face) {
	assert(face.number_of_holes() == 0);
	auto ccb_start = face.outer_ccb();
	auto ccb = ccb_start;

	std::vector<CSTraits::X_monotone_curve_2> x_monotone_curves;
	do {
		auto curve = ccb->curve();
		x_monotone_curves.push_back(curve);
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
	auto bbox = poly.bbox();
	Point<Exact> point_outside(bbox.xmin() - 1, bbox.ymin() - 1);
	Point<Exact> approx_point_on_boundary = get_approx_point_on_boundary(face);
	Segment<Exact> seg(point_outside, approx_point_on_boundary);
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
		Vector<Exact> pt1_ = makeExact(approximateAlgebraic(pt1)) - CGAL::ORIGIN;
		Number<Exact> a1 = v * pt1_;
	    Vector<Exact> pt2_ = makeExact(approximateAlgebraic(pt1)) - CGAL::ORIGIN;
	    Number<Exact> a2 = v * pt2_;
		return a1 < a2;
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

DilatedPatternArrangement
dilateAndArrange(const Partition& partition, const GeneralSettings& gs, const ComputeDrawingSettings& cds) {
	std::vector<CSPolygon> dilated;
	for (const auto& p : partition) {
		dilated.push_back(dilatePattern(*p, gs.dilationRadius()));
	}

	DilatedPatternArrangement arr;
//	CSTraits traits;
//	auto make_x_monotone = traits.make_x_monotone_2_object();
	std::vector<CSTraits::X_monotone_curve_2> x_monotone_curves;
	for (const auto& poly : dilated) {
		std::copy(poly.curves_begin(), poly.curves_end(), std::back_inserter(x_monotone_curves));
	}
	CGAL::insert(arr, x_monotone_curves.begin(), x_monotone_curves.end());

	for (auto fit = arr.faces_begin(); fit != arr.faces_end(); ++fit) {
		if (fit->is_unbounded()) continue;
		auto pt = get_point_in(*fit);
		std::vector<int> origins;
		for (int i = 0; i < dilated.size(); i++) {
			if (on_or_inside(dilated[i], pt)) {
				origins.push_back(i);
			}
//			dilated[i].curves_begin()->
		}
		fit->set_data(FaceInfo{origins});
	}

	return arr;
}

ArrangementPainting::ArrangementPainting(const DilatedPatternArrangement& arr, const DrawSettings& ds,
                                         const Partition& partition)
    : m_arr(arr), m_ds(ds), m_partition(partition) {};

void ArrangementPainting::paint(renderer::GeometryRenderer& renderer) const {
	renderer.setMode(renderer::GeometryRenderer::fill);
	for (auto fit = m_arr.faces_begin(); fit != m_arr.faces_end(); ++fit) {
		auto face = *fit;
		auto origins = face.data().origins;
		if (origins.size() > 1) {
			renderer.setFill(Color{0, 0, 0});
		} else if (origins.size() == 1) {
			renderer.setFill(Color{0, 0, 0});
//			m_ds.colors[origins[0]]
			renderer.setFill(m_ds.colors[m_partition[origins[0]]->category()]);
		} else {
			continue;
		}
		auto poly = face_to_polygon(face);
		for (auto cit = poly.curves_begin(); cit != poly.curves_end(); ++cit) {
			if (cit->is_linear()) {
				renderer.draw(Segment<Inexact>(approximateAlgebraic(cit->source()), approximateAlgebraic(cit->target())));
			} else if (cit->is_circular()){
				auto circle = cit->supporting_circle();
				renderer.draw(circle);
				// todo: draw arc
			}
		}
	}

	renderer.setMode(renderer::GeometryRenderer::stroke);
	renderer.setStroke(Color{0, 0, 0}, 3.0);
	for (auto eit = m_arr.edges_begin(); eit != m_arr.edges_end(); ++eit) {
		auto curve = eit->curve();
		if (curve.is_linear()) {
			renderer.draw(Segment<Inexact>(approximateAlgebraic(curve.source()), approximateAlgebraic(curve.target())));
		} else if (curve.is_circular()){
			auto circle = curve.supporting_circle();
			renderer.draw(circle);
			// todo: draw arc
		}
	}
}
}