#include "disk_area.h"
#include "../core/arrangement_helpers.h"
#include "../core/cs_polygon_helpers.h"
#include <CGAL/Arr_circle_segment_traits_2.h>

#include <CGAL/Boolean_set_operations_2.h>

namespace cartocrow::chorematic_map {

// Somehow using this results in a segmentation fault, though copying the function body works.
Number<Inexact> totalWeight(const Circle<Inexact>& diskInexact, std::shared_ptr<RegionArrangement> arr,
						  const std::unordered_map<std::string, double>& regionWeights) {
	Circle<Exact> disk = makeExact(diskInexact);
	auto circleCS = circleToCSPolygon(disk);

	Number<Inexact> total = 0;
	for (auto fit = arr->faces_begin(); fit != arr->faces_end(); ++fit) {
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
}
