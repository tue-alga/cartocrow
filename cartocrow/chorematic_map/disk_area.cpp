#include "disk_area.h"
#include "../core/arrangement_helpers.h"
#include "cartocrow/circle_segment_helpers/cs_curve_helpers.h"
#include "cartocrow/circle_segment_helpers/cs_polygon_helpers.h"
#include <CGAL/Arr_circle_segment_traits_2.h>

#include <CGAL/Boolean_set_operations_2.h>

namespace cartocrow::chorematic_map {
Number<Inexact> totalWeight(const Circle<Exact>& disk, const RegionArrangement& arr,
						    const std::unordered_map<std::string, double>& regionWeights) {
	CSPolygon circleCS = circleToCSPolygon(disk);
	if (circleCS.orientation() == CGAL::CLOCKWISE) {
		circleCS.reverse_orientation();
	}

	Number<Inexact> total = 0;
	for (auto fit = arr.faces_begin(); fit != arr.faces_end(); ++fit) {
		if (fit->is_unbounded()) continue;
		auto w = regionWeights.contains(fit->data()) ? regionWeights.at(fit->data()) : 0;
		if (w == 0) continue;
		auto pwh = face_to_polygon_with_holes<Exact>(fit);
		CSPolygonWithHoles pwhCS = polygonToCSPolygon(pwh);
		std::vector<CSPolygonWithHoles> inters;
		CGAL::intersection(circleCS, pwhCS, std::back_inserter(inters));
		for (const auto& inter : inters) {
			total += abs(area(inter)) * w;
		}
	}
	return total;
}
Number<Inexact> totalWeight(const GeneralCircle<Exact>& gDisk, const RegionArrangement& arr,
							const std::unordered_map<std::string, double>& regionWeights) {
	if (gDisk.is_circle()) {
		return totalWeight(gDisk.get_circle(), arr, regionWeights);
	} else {
		Halfplane<Exact> hp = gDisk.get_halfplane();
		Number<Exact> xmin = arr.vertices_begin()->point().x();
		Number<Exact> xmax = arr.vertices_begin()->point().x();
		Number<Exact> ymin = arr.vertices_begin()->point().y();
		Number<Exact> ymax = arr.vertices_begin()->point().y();
		for (auto vit = arr.vertices_begin(); vit != arr.vertices_end(); ++vit) {
			if (vit->point().x() > xmax) {
				xmax = vit->point().x();
			}
			if (vit->point().x() < xmin) {
				xmin = vit->point().x();
			}
			if (vit->point().y() > ymax) {
				ymax = vit->point().y();
			}
			if (vit->point().y() < ymin) {
				ymin = vit->point().y();
			}
		}
		Polygon<Exact> halfplanePoly = hp.polygon(Rectangle<Exact>(xmin, ymin, xmax, ymax));

		Number<Inexact> total = 0;
		for (auto fit = arr.faces_begin(); fit != arr.faces_end(); ++fit) {
			if (fit->is_unbounded()) continue;
			auto w = regionWeights.contains(fit->data()) ? regionWeights.at(fit->data()) : 0;
			if (w == 0) continue;
			auto pwh = face_to_polygon_with_holes<Exact>(fit);
			std::vector<PolygonWithHoles<Exact>> inters;
			CGAL::intersection(halfplanePoly, pwh, std::back_inserter(inters));
			for (const auto& inter : inters) {
				total += abs(approximate(inter.outer_boundary()).area()) * w;
				for (const auto& h : inter.holes()) {
					total -= abs(approximate(h).area()) * w;
				}
			}
		}
		return total;
	}
}
}
