#include "partition_algorithm.h"
#include "dilated/dilated_poly.h"

#include <queue>

#include <CGAL/Boolean_set_operations_2.h>
#include "helpers/cs_polygon_helpers.h"

namespace cartocrow::simplesets {

std::variant<Bank, Island> to_bank_or_island(PolyPattern* polyPattern) {
	if (auto bp = dynamic_cast<Bank*>(polyPattern)) {
		return *bp;
	} else if (auto ip = dynamic_cast<Island*>(polyPattern)) {
		return *ip;
	} else if (auto spp = dynamic_cast<SinglePoint*>(polyPattern)) {
		return Bank(spp->catPoints());
	} else if (auto mp = dynamic_cast<Matching*>(polyPattern)) {
		return Bank(mp->catPoints());
	} else {
		throw std::runtime_error("Unknown PolyPattern.");
	}
}

Number<Inexact> squared_distance(
    const std::variant<Polyline<Inexact>, Polygon<Inexact>>& contour,
    const Point<Inexact>& p) {
	return std::visit([&p](auto& poly) {
		Number<Inexact> min_sqrd_dist = std::numeric_limits<double>::infinity();

		for (auto eit = poly.edges_begin(); eit != poly.edges_end(); ++eit) {
			auto seg = *eit;
			auto dist = CGAL::squared_distance(seg, p);
			if (dist < min_sqrd_dist) {
				min_sqrd_dist = dist;
			}
		}

		return min_sqrd_dist;
	}, contour);
}

bool is_inside(const Point<Inexact>& point, const Polygon<Inexact>& polygon) {
	return !polygon.has_on_unbounded_side(point);
}

bool is_inside(const Point<Inexact>& point, const Polyline<Inexact>& polygon) {
	return false;
}

bool do_intersect(
    const std::variant<Polyline<Inexact>, Polygon<Inexact>>& cont1,
	const std::variant<Polyline<Inexact>, Polygon<Inexact>>& cont2) {
	return std::visit([&cont2](auto& poly1) {
		return std::visit([&poly1](auto& poly2) {
			for (auto eit = poly1.edges_begin(); eit != poly1.edges_end(); ++eit) {
				for (auto eitt = poly2.edges_begin(); eitt != poly2.edges_end(); ++eitt) {
					if (CGAL::do_intersect(*eit, *eitt)) {
						return true;
					}
				}
			}

			return is_inside(poly1.vertex(0), poly2) || is_inside(poly2.vertex(0), poly1);
		}, cont2);
	}, cont1);
}

Number<Inexact> intersectionDelay(const std::vector<CatPoint>& points, const PolyPattern& p1, const PolyPattern& p2,
                                  const PolyPattern& result, const GeneralSettings& gs, const PartitionSettings& ps) {
	// todo: check consistency with paper
	if (!ps.intersectionDelay) return 0;
	Number<Inexact> intersectionArea = 0;
	auto& resultPts = result.catPoints();
	auto resultPoly = result.poly();
	for (const auto& pt : points) {
		if (std::find(resultPts.begin(), resultPts.end(), pt) == resultPts.end() &&
		    squared_distance(resultPoly, pt.point) < squared(2 * gs.dilationRadius())) {
			CSPolygon ptShape = Dilated(SinglePoint(pt), gs.dilationRadius()).m_contour;
			CSPolygon rShape = Dilated(result, gs.dilationRadius()).m_contour;
			std::vector<CSPolygonWithHoles> inters;
			CGAL::intersection(rShape, ptShape, std::back_inserter(inters));
			Number<Inexact> newArea = 0;
			for (const auto& gp : inters) {
				newArea += abs(area(gp));
			}
			inters.clear();
			CSPolygon p1Shape = Dilated(p1, gs.dilationRadius()).m_contour;
			CSPolygon p2Shape = Dilated(p2, gs.dilationRadius()).m_contour;
			CGAL::intersection(p1Shape, ptShape, std::back_inserter(inters));
			CGAL::intersection(p2Shape, ptShape, std::back_inserter(inters));
			Number<Inexact> oldArea = 0;
			for (const auto& gp : inters) {
				oldArea += abs(area(gp));
			}
			intersectionArea += newArea - oldArea;
		}
	}

	if (intersectionArea <= 0) {
		return 0;
	}
	return sqrt(intersectionArea / M_PI);
}

std::vector<std::pair<Number<Inexact>, Partition>>
partition(const std::vector<CatPoint>& points, const GeneralSettings& gs, const PartitionSettings& ps, Number<Inexact> maxTime) {
	// Create initial partition consisting of single points
	std::vector<std::shared_ptr<PolyPattern>> initialPatterns;
	std::transform(points.begin(), points.end(), std::back_inserter(initialPatterns), [](const auto& pt)
	               { return std::make_shared<SinglePoint>(pt); });
	Partition partition(initialPatterns);

	std::vector<std::pair<Number<Inexact>, Partition>> history;
	history.emplace_back(0, partition);

	// Priority queue storing events based on their time
	auto comparison = [](const PossibleMergeEvent& e1, const PossibleMergeEvent& e2) { return e1.time > e2.time; };
	std::priority_queue<PossibleMergeEvent, std::vector<PossibleMergeEvent>, decltype(comparison)> events{comparison};

	// Add SinglePoint--SinglePoint merges
	for (int i = 0; i < partition.size(); ++i) {
		auto& p = dynamic_cast<SinglePoint&>(*partition[i]);
		for (int j = i + 1; j < partition.size(); ++j) {
			auto& q = dynamic_cast<SinglePoint&>(*partition[j]);
			if (p.category() != q.category()) continue;
			if (squared_distance(p.catPoint().point, q.catPoint().point) > squared(2 * maxTime)) continue;

			auto newPattern = std::make_shared<Matching>(p.catPoint(), q.catPoint());
			Segment<Inexact> seg(p.catPoint().point, q.catPoint().point);

			bool tooClose = false;
			for (const CatPoint& pt : points) {
				// Check if a point is too close to the segment
				if (pt != p.catPoint() && pt != q.catPoint() &&
				    CGAL::squared_distance(seg, pt.point) < squared(ps.admissibleRadiusFactor * gs.dilationRadius()) &&
				    CGAL::squared_distance(seg, pt.point) < CGAL::min(CGAL::squared_distance(p.catPoint().point, pt.point),
				                                                CGAL::squared_distance(q.catPoint().point, pt.point)) - M_EPSILON) {
					tooClose = true;
					break;
				}
			}
			if (tooClose) continue;

			PossibleMergeEvent event{newPattern->coverRadius(), partition[i], partition[j], newPattern, false};
			events.push(event);
		}
	}

	while (!events.empty()) {
		auto ev = events.top();
		events.pop();

		if (ev.time > maxTime) break;

		if (!ev.final) {
			auto delay = intersectionDelay(points, *ev.p1, *ev.p2, *ev.result, gs, ps);
			ev.time += delay;
			ev.final = true;
			events.push(ev);
			continue;
		}

		// Check if patterns that events wants to merge still exist
		bool foundP1 = false;
		bool foundP2 = false;
		for (const auto& pattern : partition) {
			if (pattern == ev.p1) {
				foundP1 = true;
			}
			if (pattern == ev.p2) {
				foundP2 = true;
			}
		}
		if (!foundP1 || !foundP2) continue;

		auto& newPts = ev.result->catPoints();
		auto newPoly = ev.result->poly();

		// Check for intersections with existing patterns
		bool intersects = false;
		for (const auto& pattern : partition) {
			if (pattern != ev.p1 && pattern != ev.p2 && do_intersect(pattern->poly(), newPoly)) {
				intersects = true;
				break;
			}
		}
		if (intersects) continue;

		bool tooClose = false;
		// Check whether any point is too close to the result pattern
		for (const auto& pt : points) {
			if (std::find(newPts.begin(), newPts.end(), pt) == newPts.end()) {
			    auto polyPtDist = squared_distance(newPoly, pt.point);
				std::optional<Number<Inexact>> pointPtDist;
				for (auto np : newPts) {
					auto d = CGAL::squared_distance(np.point, pt.point);
					if (!pointPtDist.has_value() || d < *pointPtDist) {
						pointPtDist = d;
					}
				}
				if (polyPtDist < squared(ps.admissibleRadiusFactor * gs.dilationRadius()) &&
				    polyPtDist < pointPtDist) {
					tooClose = true;
					break;
				}
			}
		}
		if (tooClose) continue;

		// Merge
		// Remove the two patterns that are merged
		auto startTrash = std::remove_if(partition.begin(), partition.end(),
		                                 [&ev](const auto& part) { return part == ev.p1 || part == ev.p2; });
		partition.erase(startTrash, partition.end());
		// Add the result
		partition.push_back(ev.result);
		// Save this partition
		history.emplace_back(ev.time, partition);

		// Create new merge events
		for (const auto& pattern : partition) {
			if (pattern == ev.result || pattern->category() != ev.result->category()) continue;

			if (ps.islands) {
				// Do relatively cheap check first: are the points in the two patterns close enough to ever form a pattern?
				Number<Inexact> min_sqrd_dist = std::numeric_limits<double>::infinity();
				for (const auto& p : pattern->catPoints()) {
					for (const auto& q : newPts) {
						auto dist = squared_distance(p.point, q.point);
						if (dist < min_sqrd_dist) {
							min_sqrd_dist = dist;
						}
					}
				}

				if (min_sqrd_dist <= squared(2 * maxTime)) {
					std::vector<CatPoint> mergedPoints;
					std::copy(newPts.begin(), newPts.end(), std::back_inserter(mergedPoints));
					std::copy(pattern->catPoints().begin(), pattern->catPoints().end(), std::back_inserter(mergedPoints));
					auto newIsland = std::make_shared<Island>(mergedPoints);

					// todo? do intersection check here already? check how this affects performance

					Number<Inexact> regDelay = !ps.regularityDelay ? 0 : newIsland->coverRadius() - std::max(pattern->coverRadius(), ev.result->coverRadius());
					Number<Inexact> eventTime = newIsland->coverRadius() + regDelay;
					PossibleMergeEvent newEvent{eventTime, ev.result, pattern, newIsland, false};
					if (eventTime <= maxTime) {
						events.push(newEvent);
					}
				}
			}

			if (ps.banks) {
				auto pat1 = to_bank_or_island(&*pattern);
				auto pat2 = to_bank_or_island(&*ev.result);

				if (std::holds_alternative<Bank>(pat1) && std::holds_alternative<Bank>(pat2)) {
					Bank bank1 = std::get<Bank>(pat1);
					Bank bank2 = std::get<Bank>(pat2);
					auto pts1 = bank1.catPoints();
					auto pts2 = bank2.catPoints();

					std::vector<CatPoint> b1Pts;
					std::copy(pts1.begin(), pts1.end(), std::back_inserter(b1Pts));
					std::copy(pts2.begin(), pts2.end(), std::back_inserter(b1Pts));
					auto b1 = std::make_shared<Bank>(b1Pts);

					std::vector<CatPoint> b2Pts;
					std::copy(pts1.begin(), pts1.end(), std::back_inserter(b2Pts));
					std::copy(pts2.rbegin(), pts2.rend(), std::back_inserter(b2Pts));
					auto b2 = std::make_shared<Bank>(b2Pts);

					std::vector<CatPoint> b3Pts;
					std::copy(pts1.rbegin(), pts1.rend(), std::back_inserter(b3Pts));
					std::copy(pts2.rbegin(), pts2.rend(), std::back_inserter(b3Pts));
					auto b3 = std::make_shared<Bank>(b3Pts);

					std::vector<CatPoint> b4Pts;
					std::copy(pts1.rbegin(), pts1.rend(), std::back_inserter(b4Pts));
					std::copy(pts2.begin(), pts2.end(), std::back_inserter(b4Pts));
					auto b4 = std::make_shared<Bank>(b4Pts);

					for (auto& b : {b1, b2, b3, b4}) {
						if (!b->isValid(gs)) continue;
						Number<Inexact> regDelay = !ps.regularityDelay ? 0 : b->coverRadius() - std::max(ev.result->coverRadius(), pattern->coverRadius());
						Number<Inexact> eventTime = b->coverRadius() + regDelay;
						PossibleMergeEvent newEvent{eventTime * 1, ev.result, pattern, b, false};
						if (eventTime <= maxTime) {
							events.push(newEvent);
						}
					}
				}
			}
		}
	}

	return history;
}
}