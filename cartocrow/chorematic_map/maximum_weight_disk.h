#ifndef CARTOCROW_MAXIMUM_WEIGHT_DISK_H
#define CARTOCROW_MAXIMUM_WEIGHT_DISK_H

#include "../core/core.h"
#include "../renderer/ipe_renderer.h"
#include "../renderer/geometry_renderer.h"

namespace cartocrow::chorematic_map {
struct WeightedPoint {
	Point<Inexact> point;
	Number<Inexact> weight;
};

/// Based on the paper:
/// Smallest Maximum-Weight Circle for Weighted Points in the Plane
/// by Sergey Bereg, Ovidiu Daescu, Marko Zivanic, and Timothy Rozario
template <class InputIterator>
std::tuple<WeightedPoint, WeightedPoint, WeightedPoint> maximum_weight_disk(InputIterator begin, InputIterator end) {
	// Positive weight points
	std::vector<WeightedPoint> pos;
	// Negative weight points
	std::vector<WeightedPoint> neg;

	renderer::IpeRenderer ipeRenderer;
	ipeRenderer.addPainting([begin, end](renderer::GeometryRenderer& renderer) {
	  	for (auto it = begin; it != end; ++it) {
			if (it->weight > 0) {
				renderer.setFill(0xFF0000);
				renderer.setStroke(0xFF0000, 1.0);
				renderer.draw(it->point);
			}
			if (it->weight < 0) {
				renderer.setFill(0x0000FF);
				renderer.setStroke(0x0000FF, 1.0);
				renderer.draw(it->point);
			}
		}
	}, "Points");

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

	double overallBestWeight = 0.0;
	std::tuple<WeightedPoint, WeightedPoint, WeightedPoint> overallBestTriple(*begin, *begin, *begin);

	for (int i = 0; i < pos.size(); ++i) {
		for (int j = i + 1; j < pos.size(); ++j) {
//	int i = 0;
//	int j = 1;
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
				if (side != CGAL::ON_UNBOUNDED_SIDE) {
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

//			ipeRenderer.addPainting([lij, bestCandidate, pi, pj, candidates](renderer::GeometryRenderer& renderer) {
//				renderer.draw(lij);
//		        for (int index = 0; index < candidates.size(); ++index) {
//			        auto c = candidates[index];
//			        int gray = 255 * index / candidates.size();
//			        renderer.setStroke(Color{gray, gray, gray}, 1.0);
//			        renderer.draw(c.point);
//
//		        }
//				Circle<Inexact> circle(pi.point, pj.point, bestCandidate.point);
//				renderer.draw(circle);
//			}, "Bisector_sorted");
//
//	        ipeRenderer.save("well.ipe");
		}
	}

	return overallBestTriple;
}
}

#endif //CARTOCROW_MAXIMUM_WEIGHT_DISK_H
