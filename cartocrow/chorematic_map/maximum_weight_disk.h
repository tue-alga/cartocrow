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

	int nPoints = 0;

	for (auto it = begin; it != end; ++it) {
		++nPoints;
		WeightedPoint pt = *it;
		if (pt.weight < 0) {
			neg.push_back(pt);
		}
		if (pt.weight > 0) {
			pos.push_back(pt);
		}
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

	auto task = [nPoints, &pos, &begin, &end](int iStart, int iEnd) {
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

				std::vector<std::tuple<int, WeightedPoint, Point<Inexact>>> negCandidates;
				std::vector<std::tuple<int, WeightedPoint, Point<Inexact>>> posCandidates;

				int index = -1;
				for (auto qit = begin; qit != end; ++qit) {
					++index;
					WeightedPoint q = *qit;
					if (q.point == pi.point || q.point == pj.point) continue;
					// Edge case: points are (roughly) collinear.
					// A smallest maximum weight circle cannot be defined by three collinear points.
					if (abs(CGAL::area(pi.point, pj.point, q.point)) < 0.001) {
						continue;
					}
					auto cc = CGAL::circumcenter(pi.point, pj.point, q.point);
					auto ori = CGAL::orientation(pi.point, pj.point, cc);
					if (ori == CGAL::POSITIVE) {
						posCandidates.emplace_back(index, q, cc);
					} else if (ori == CGAL::NEGATIVE) {
						negCandidates.emplace_back(index, q, cc);
					}
				}

				for (CGAL::Sign side : {CGAL::NEGATIVE, CGAL::POSITIVE}) {
					auto& candidates = side == CGAL::NEGATIVE ? negCandidates : posCandidates;
					std::sort(
						candidates.begin(), candidates.end(),
						[pi, pj, vij, m](const std::tuple<int, WeightedPoint, Point<Inexact>>& wp1, const std::tuple<int, WeightedPoint, Point<Inexact>>& wp2) {
							return abs((get<2>(wp1) - m) * vij) < abs((get<2>(wp2) - m) * vij);
						});

					std::vector<bool> inDisk(nPoints);
					double totalWeight = 0;
					int index1 = -1;
					for (auto qit = begin; qit != end; ++qit) {
						++index1;
						auto& q = *qit;
						auto sideOfCircle =
							CGAL::side_of_bounded_circle(pi.point, pj.point, q.point);
						if (sideOfCircle != CGAL::ON_UNBOUNDED_SIDE) {
							inDisk[index1] = true;
							totalWeight += q.weight;
						} else {
							inDisk[index1] = false;
						}
					}

					double bestTotalWeight = totalWeight;
					double sqRadiusOfBest = CGAL::squared_distance(pi.point, m);
					std::optional<WeightedPoint> bestCandidate = std::nullopt;
					for (auto& [index2, cand, cc] : candidates) {
						if (inDisk[index2]) {
							totalWeight -= cand.weight;
						} else {
							totalWeight += cand.weight;
						}
						inDisk[index2] = !inDisk[index2];

						// Invariant: the smallest disk that contains the points of indices that inDisk sets to true with pi and pj on the boundary
						// is defined by pi, pj, and cand.

						// Point with negative weight that lies on the boundary can be "chosen" to lie on either side of the disk.
						double sqRadius = CGAL::squared_distance(pi.point, cc);

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

	bool useParallel = nPoints > 32;

	std::vector<std::future<Result>> results;
	if (useParallel) {
		int n = pos.size();
		int nThreads = std::min(128, n);
		double step = n / static_cast<double>(nThreads);
		for (int i = 0; i < n / step; ++i) {
			int iStart = std::ceil(i * step);
			int iEnd = std::ceil((i + 1) * step);
			results.push_back(std::async(std::launch::async, task, iStart, iEnd));
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
