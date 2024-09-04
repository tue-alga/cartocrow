#include "grow_circles.h"

namespace cartocrow::simplesets {
std::pair<std::vector<Circle<Exact>>, std::vector<Circle<Exact>>>
approximateGrowCircles(const std::vector<Point<Exact>>& points1,
                       const std::vector<Point<Exact>>& points2,
                       const Number<Exact>& maxSquaredRadius1,
                       const Number<Exact>& maxSquaredRadius2) {
	std::vector<GrowingCircle> growingCircles1;
	std::transform(points1.begin(), points1.end(), std::back_inserter(growingCircles1), [](const Point<Exact>& pt) {
		return GrowingCircle{pt, 0};
	});
	std::vector<GrowingCircle> growingCircles2;
	std::transform(points2.begin(), points2.end(), std::back_inserter(growingCircles2), [](const Point<Exact>& pt) {
	    return GrowingCircle{pt, 0};
	});

	// if statement and while(true) combined.
	while (!growingCircles1.empty() && !growingCircles2.empty()) {
		 bool done = true;
		 for (const auto& c : growingCircles1) {
			 if (!c.frozen) {
				 done = false;
				 break;
			 }
		 }
		 for (const auto& c : growingCircles2) {
			 if (!c.frozen) {
				 done = false;
				 break;
			 }
		 }
		if (done) break;

		// Note that all variables representing distances contain squared distances.
		std::optional<Number<Exact>> minDist;
		std::optional<std::pair<GrowingCircle*, GrowingCircle*>> minPair;
		for (auto& c1 : growingCircles1) {
			for (auto& c2 : growingCircles2) {
				if (c1.frozen && c2.frozen) continue;
				auto centerDist = squared_distance(c1.center, c2.center);

				Number<Exact> growDist;
				if (!c1.frozen && !c2.frozen) {
					growDist = centerDist / 4;
				} else if (c1.frozen) {
					growDist = centerDist + c1.squaredRadius - 2 * sqrt(CGAL::to_double(centerDist)) * sqrt(CGAL::to_double(c1.squaredRadius));
				} else {
					growDist = centerDist + c2.squaredRadius - 2 * sqrt(CGAL::to_double(centerDist)) * sqrt(CGAL::to_double(c2.squaredRadius));
				}

				if (!minDist.has_value() || growDist < *minDist) {
					minDist = growDist;
					minPair = {&c1, &c2};
				}
			}
		}

		auto& [c1, c2] = *minPair;

		if (!c1->frozen && !c2->frozen) {
			auto d = std::min(*minDist, std::min(maxSquaredRadius1, maxSquaredRadius2));
			if (d == *minDist) {
				c1->frozen = true;
				c2->frozen = true;
			} else if (d == maxSquaredRadius1) {
				c1->frozen = true;
			} else {
				assert(d == maxSquaredRadius2);
				c2->frozen = true;
			}
			c1->squaredRadius = d;
			c2->squaredRadius = d;
		} else if (c1->frozen) {
			c2->squaredRadius = std::min(*minDist, maxSquaredRadius2);
			c2->frozen = true;
		} else {
			assert(c2->frozen);
			c1->squaredRadius = std::min(*minDist, maxSquaredRadius1);
			c1->frozen = true;
		}
	}
	// Trivial case
	if(growingCircles1.empty() || growingCircles2.empty()) {
		for (auto& c : growingCircles1) {
			c.squaredRadius = maxSquaredRadius1;
		}
		for (auto& c : growingCircles2) {
			c.squaredRadius = maxSquaredRadius2;
		}
	}

	std::vector<Circle<Exact>> circles1;
	std::vector<Circle<Exact>> circles2;
	std::transform(growingCircles1.begin(), growingCircles1.end(), std::back_inserter(circles1),
	               [](const GrowingCircle& gc) { return Circle<Exact>(gc.center, gc.squaredRadius); });
	std::transform(growingCircles2.begin(), growingCircles2.end(), std::back_inserter(circles2),
				   [](const GrowingCircle& gc) { return Circle<Exact>(gc.center, gc.squaredRadius); });

	return {circles1, circles2};
}
}