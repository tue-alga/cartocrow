#ifndef CARTOCROW_MAXIMUM_WEIGHT_DISK_H
#define CARTOCROW_MAXIMUM_WEIGHT_DISK_H

#include "weighted_point.h"

namespace cartocrow::chorematic_map {
/// Based on the paper:
/// Smallest Maximum-Weight Circle for Weighted Points in the Plane
/// by Sergey Bereg, Ovidiu Daescu, Marko Zivanic, and Timothy Rozario
template <class InputIterator>
InducedDiskW maximum_weight_disk(InputIterator begin, InputIterator end) {
	// Positive weight points
	std::vector<WeightedPoint> pos;
	// Negative weight points
	std::vector<WeightedPoint> neg;

	for (auto it = begin; it != end; ++it) {
		WeightedPoint pt = *it;
		if (pt.weight < 0) {
			neg.push_back(pt);
		}
		if (pt.weight > 0) {
			pos.push_back(pt);
		}
		// We ignore zero-weight points
	}

	if (pos.size() == 1) {
		return {pos[0], std::nullopt, std::nullopt};
	}

	double overallBestWeight = 0.0;
	std::tuple<WeightedPoint, WeightedPoint, WeightedPoint> overallBestTriple(*begin, *begin, *begin);

	for (int i = 0; i < pos.size(); ++i) {
		for (int j = i + 1; j < pos.size(); ++j) {
			WeightedPoint pi = pos[i];
			WeightedPoint pj = pos[j];

			std::vector<WeightedPoint> candidates;
			for (auto qit = begin; qit != end; ++qit) {
				WeightedPoint q = *qit;
				// Edge case, for now do not consider degenerate circles that are straight lines.
				if (CGAL::collinear(pi.point, pj.point, q.point)) continue;
				candidates.push_back(q);
			}

			auto lij = CGAL::bisector(pi.point, pj.point);
			auto vij = lij.to_vector();
			auto m = CGAL::midpoint(pi.point, pj.point);
			std::sort(candidates.begin(), candidates.end(), [pi, pj, vij, m](const WeightedPoint& wp1, const WeightedPoint& wp2) {
				return (CGAL::circumcenter(pi.point, pj.point, wp1.point) - m) * vij < (CGAL::circumcenter(pi.point, pj.point, wp2.point) - m) * vij;
			});

			std::vector<bool> inDisk(candidates.size());
			double totalWeight = 0;
			for (int index = 0; index < candidates.size(); ++index) {
				auto& p = candidates[index];
				auto side = CGAL::side_of_bounded_circle(candidates[0].point, pi.point, pj.point, p.point);
				if (side == CGAL::ON_BOUNDED_SIDE) {
					inDisk[index] = true;
					totalWeight += p.weight;
				}
				if (side != CGAL::ON_BOUNDARY && p.weight > 0) {
					inDisk[index] = true;
					totalWeight += p.weight;
				}
			}

			double bestTotalWeight = totalWeight;
			WeightedPoint bestCandidate = candidates[0];
			for (int index = 0; index < inDisk.size(); ++index) {
				auto& cand = candidates[index];
				if (inDisk[index]) {
					totalWeight -= cand.weight;
				} else {
					totalWeight += cand.weight;
				}
				inDisk[index] = !inDisk[index];

				if (totalWeight > bestTotalWeight) {
					bestTotalWeight = totalWeight;
					bestCandidate = cand;
				}
			}

			if (bestTotalWeight > overallBestWeight) {
				overallBestWeight = bestTotalWeight;
				overallBestTriple = {pi, pj, bestCandidate};
			}
		}
	}

	return overallBestTriple;
}

template <class InputIterator>
InducedDiskW minimum_weight_disk(InputIterator begin, InputIterator end) {
	std::vector<WeightedPoint> invertedWeights;
	std::copy(begin, end, std::back_inserter(invertedWeights));
	for (auto& pt : invertedWeights) {
		pt.weight = -pt.weight;
	}
	return maximum_weight_disk(invertedWeights.begin(), invertedWeights.end());
}
}

#endif //CARTOCROW_MAXIMUM_WEIGHT_DISK_H
