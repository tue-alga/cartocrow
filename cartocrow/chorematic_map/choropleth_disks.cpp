#include "choropleth_disks.h"
#include "maximum_weight_disk.h"

namespace cartocrow::chorematic_map {
/// The disks are returned in the order that they should be drawn.
std::vector<BinDisk> fitDisks(const Choropleth& choropleth, const WeightedRegionSample<Exact>& sample, bool invert) {
	using RegionWeight = WeightedRegionSample<Exact>::RegionWeight;

	std::vector<int> binsToFit;
	if (invert) {
		for (int binToFit = 0; binToFit < choropleth.numberOfBins() - 1; ++binToFit) {
			binsToFit.push_back(binToFit);
		}
	} else {
		for (int binToFit = choropleth.numberOfBins() - 1; binToFit >= 1; --binToFit) {
			binsToFit.push_back(binToFit);
		}
	}

	std::vector<BinDisk> binDisks;
	for (int binToFit : binsToFit) {
		// Compute weights for this bin
		RegionWeight regionWeight;
		Number<Exact> negativeArea = 0;
		auto binAreas = choropleth.binAreas();
		for (int i = 0; i < binAreas.size(); ++i) {
			if (i == binToFit) continue;
			auto& binArea = binAreas[i];
			negativeArea += binArea;
		}
		auto& positiveArea = binAreas[binToFit];
		for (const auto& [region, _] : *(choropleth.m_data)) {
			auto bin = choropleth.regionToBin(region);
			if (!bin.has_value()) continue;

			regionWeight[region] = CGAL::to_double(*bin == binToFit ? negativeArea : -positiveArea);
		}

		std::vector<WeightedPoint> weightedPoints;
		sample.weightedPoints(std::back_inserter(weightedPoints), regionWeight);

		if (!binDisks.empty()) {
			auto it = std::remove_if(
			    weightedPoints.begin(), weightedPoints.end(), [&binDisks](const WeightedPoint& wp) {
				    for (const auto& disk : binDisks) {
					    const auto& [p1, p2, p3] = disk.disk;
					    if (p1.has_value() && p2.has_value() && p3.has_value()) {
						    if (CGAL::side_of_bounded_circle(p1->point, p2->point, p3->point,
						                                     wp.point) != CGAL::ON_UNBOUNDED_SIDE) {
							    return true;
						    }
					    } else if (p1.has_value() && p2.has_value()) {
						    if (CGAL::side_of_bounded_circle(p1->point, p2->point, wp.point) !=
						        CGAL::ON_UNBOUNDED_SIDE) {
							    return true;
						    }
					    } else if (p1.has_value()) {
						    if (wp.point == p1->point) {
							    return true;
						    }
					    }
				    }
				    return false;
			    });
			weightedPoints.erase(it, weightedPoints.end());
		}

		InducedDiskW iDisk = smallest_maximum_weight_disk(weightedPoints.begin(), weightedPoints.end());
		binDisks.emplace_back(binToFit, iDisk);
	}

	std::reverse(binDisks.begin(), binDisks.end());
	return binDisks;
}
}