#ifndef CARTOCROW_MAXIMUM_WEIGHT_DISK_H
#define CARTOCROW_MAXIMUM_WEIGHT_DISK_H

#include "weighted_point.h"
#include <future>

namespace cartocrow::chorematic_map {
/// Based on the paper:
/// Smallest Maximum-Weight Circle for Weighted Points in the Plane
/// by Sergey Bereg, Ovidiu Daescu, Marko Zivanic, and Timothy Rozario
template <class InputIterator>
InducedDiskW smallest_maximum_weight_disk(InputIterator begin, InputIterator end,
                                          std::optional<std::function<void(int)>> progress = std::nullopt,
                                          std::optional<std::function<bool()>> cancelled = std::nullopt) {
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

	if (pos.empty()) {
		return {std::nullopt, std::nullopt, std::nullopt};
	}

	// Positive weight points
	if (pos.size() == 1) {
		return {pos[0], std::nullopt, std::nullopt};
	}

	struct Result {
		double bestWeight;
		double squaredRadius;
		InducedDiskW disk;
	};

	auto task = [&pos, &begin, &end](int iStart, int iEnd) {
	  double localBestWeight = 0.0;
	  double localSquaredRadius = 0.0;
	  InducedDiskW localBestTriple(std::nullopt, std::nullopt, std::nullopt);
		for (int i = iStart; i < iEnd; ++i) {
			for (int j = i + 1; j < pos.size(); ++j) {
				WeightedPoint pi = pos[i];
				WeightedPoint pj = pos[j];

				auto lij = CGAL::bisector(pi.point, pj.point);
				auto vij = lij.to_vector();
				auto m = CGAL::midpoint(pi.point, pj.point);

				for (CGAL::Sign side : {CGAL::NEGATIVE, CGAL::POSITIVE}) {
					std::vector<WeightedPoint> candidates;
					for (auto qit = begin; qit != end; ++qit) {
						WeightedPoint q = *qit;
						if (CGAL::collinear(pi.point, pj.point, q.point))
							continue;
						// Edge case, for now do not consider degenerate circles that are straight lines.
						auto cc = CGAL::circumcenter(pi.point, pj.point, q.point);
						if (CGAL::orientation(pi.point, pj.point, cc) == side)
							candidates.push_back(q);
					}

					std::sort(
						candidates.begin(), candidates.end(),
						[pi, pj, vij, m](const WeightedPoint& wp1, const WeightedPoint& wp2) {
							return abs((CGAL::circumcenter(pi.point, pj.point, wp1.point) - m) *
									   vij) <
								   abs((CGAL::circumcenter(pi.point, pj.point, wp2.point) - m) *
									   vij);
						});

					// Todo: use more efficient data structure
					std::map<WeightedPoint, bool> inDisk;
					double totalWeight = 0;
					for (auto qit = begin; qit != end; ++qit) {
						auto& q = *qit;
						auto sideOfCircle =
							CGAL::side_of_bounded_circle(pi.point, pj.point, q.point);
						if (sideOfCircle != CGAL::ON_UNBOUNDED_SIDE) {
							inDisk[q] = true;
							totalWeight += q.weight;
						} else {
							inDisk[q] = false;
						}
					}

					double bestTotalWeight = totalWeight;
					double sqRadiusOfBest = CGAL::squared_distance(pi.point, m);
					std::optional<WeightedPoint> bestCandidate = std::nullopt;
					for (auto& cand : candidates) {
						if (inDisk.at(cand)) {
							totalWeight -= cand.weight;
						} else {
							totalWeight += cand.weight;
						}
						inDisk[cand] = !inDisk[cand];

						// Invariant: the smallest disk that contains the points of indices that inDisk sets to true with pi and pj on the boundary
						// is defined by pi, pj, and cand.

						// Point with negative weight that lies on the boundary can be "chosen" to lie on either side of the disk.
						double sqRadius = CGAL::squared_distance(
							pi.point, CGAL::circumcenter(pi.point, pj.point, cand.point));

						if (totalWeight > bestTotalWeight ||
							totalWeight == bestTotalWeight && sqRadius < sqRadiusOfBest) {
							bestTotalWeight = totalWeight;
							bestCandidate = cand;
							sqRadiusOfBest = sqRadius;
						}
					}

					if (bestTotalWeight > localBestWeight ||
						bestTotalWeight == localBestWeight &&
							sqRadiusOfBest < localSquaredRadius) {
						localBestWeight = bestTotalWeight;
						localBestTriple = {pi, pj, bestCandidate};
						localSquaredRadius = sqRadiusOfBest;
					}
				}
			}
		}
		return Result{localBestWeight, localSquaredRadius, localBestTriple};
	};

	double overallBestWeight = 0.0;
	double overallSquaredRadius = 0.0;
	InducedDiskW overallBestTriple(std::nullopt, std::nullopt, std::nullopt);

	for (auto & p : pos) {
		if (p.weight > overallBestWeight) {
			overallBestWeight = p.weight;
			overallBestTriple = {p, std::nullopt, std::nullopt};
		}
	}

	bool useParallel = true;

	std::vector<std::future<Result>> results;
	if (useParallel) {
		int nThreads = 32;
		int n = pos.size();
		double step = n / static_cast<double>(nThreads);
		for (int i = 0; i < n / step; ++i) {
			int iStart = std::ceil(i * step);
			int iEnd = std::ceil((i + 1) * step);
			results.push_back(std::async(task, iStart, iEnd));
		}

		for (auto& futureResult : results) {
			Result result = futureResult.get();
			if (result.bestWeight > overallBestWeight ||
			    result.bestWeight == overallBestWeight &&
			        result.squaredRadius < overallSquaredRadius) {
				overallBestWeight = result.bestWeight;
				overallSquaredRadius = result.squaredRadius;
				overallBestTriple = result.disk;
			}
		}

		return overallBestTriple;
	} else {
		return task(0, pos.size()).disk;
	}
}

template <class InputIterator>
InducedDiskW smallest_minimum_weight_disk(InputIterator begin, InputIterator end) {
	std::vector<WeightedPoint> invertedWeights;
	std::copy(begin, end, std::back_inserter(invertedWeights));
	for (auto& pt : invertedWeights) {
		pt.weight = -pt.weight;
	}
	return smallest_maximum_weight_disk(invertedWeights.begin(), invertedWeights.end());
}
}

#endif //CARTOCROW_MAXIMUM_WEIGHT_DISK_H
