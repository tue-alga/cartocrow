/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
Copyright (C) 2021  Netherlands eScience Center and TU Eindhoven

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

Created by tvl (t.vanlankveld@esciencecenter.nl) on 26-02-2021
*/

#ifndef CARTOCROW_CORE_INTERSECTIONS_H
#define CARTOCROW_CORE_INTERSECTIONS_H

#include "../core/core.h"
#include "polar_line.h"
#include "polar_point.h"
#include "polar_segment.h"
#include "spiral.h"
#include "spiral_segment.h"

namespace cartocrow::flow_map {
namespace detail {

/// Returns the side of the line that the point lies: -1: on the side of the
/// origin, 0: on the line, 1: on the side opposite to the origin.
int orientation(const PolarLine& line, const PolarPoint& point);

/// Given an interval \f$[t_1, t_2)\f$ such that a spiral intersects a line once
/// within this interval, searches for the \f$t\f$ of the intersection point
/// within this interval.
///
/// This method updates the interval \f$[t_1, t_2)\f$.
///
/// \return Whether the search was successful.
bool searchSpiralLineIntersection(const PolarLine& line, const Spiral& spiral, Number<Inexact>& t2,
                                  Number<Inexact>& t1,
                                  const Number<Inexact> t_precision = Number<Inexact>(1e-15));

/// Checks if a candidate intersection on the supporting spiral of the given
/// spiral segment actually lies on the spiral segment.
bool checkIntersection(const SpiralSegment& segment, const PolarPoint& point);
/// Checks if a candidate intersection on the supporting line of the given
/// segment actually lies on the segment.
bool checkIntersection(const PolarSegment& segment, const PolarPoint& point);

} // namespace detail

// LINE/LINE

/// Computes the intersection (if any) between the given two lines.
/// \param[out] intersections Empty list in which the intersections are placed.
void intersect(const PolarLine& line_1, const PolarLine& line_2,
               std::vector<PolarPoint>& intersections);
/// Computes the intersection (if any) between the given line and the given
/// segment.
/// \param[out] intersections Empty list in which the intersections are placed.
inline void intersect(const PolarLine& line_1, const PolarSegment& line_2,
                      std::vector<PolarPoint>& intersections) {
	std::vector<PolarPoint> lineIntersections;
	intersect(line_1, line_2.supportingLine(), lineIntersections);
	for (const PolarPoint& p : lineIntersections) {
		if (detail::checkIntersection(line_2, p)) {
			intersections.push_back(p);
		}
	}
}
/// \copydoc intersect(const PolarLine&, const PolarSegment&, std::vector<PolarPoint>&)
inline void intersect(const PolarSegment& line_1, const PolarLine& line_2,
                      std::vector<PolarPoint>& intersections) {
	intersect(line_2, line_1, intersections);
}
/// Computes the intersection (if any) between the given two segments.
/// \param[out] intersections Empty list in which the intersections are placed.
inline void intersect(const PolarSegment& line_1, const PolarSegment& line_2,
                      std::vector<PolarPoint>& intersections) {
	std::vector<PolarPoint> lineIntersections;
	intersect(line_1.supportingLine(), line_2.supportingLine(), lineIntersections);
	for (const PolarPoint& p : lineIntersections) {
		if (detail::checkIntersection(line_1, p) && detail::checkIntersection(line_2, p)) {
			intersections.push_back(p);
		}
	}
}

// SPIRAL/SPIRAL

/// Computes (at most two) intersections between the given spirals.
///
/// If the spirals intersect, this method produces the two intersections closest
/// to the first spiral's anchor and on opposite sides of that anchor. The first
/// intersection has the non-positive time closest to zero. The second
/// intersection has the positive time closest to zero.
///
/// \param[out] intersections Empty list in which the intersections are placed.
void intersect(const Spiral& spiral_1, const Spiral& spiral_2,
               std::vector<PolarPoint>& intersections);
/// Computes the intersection (if any) between the given spiral and the given
/// spiral segment.
/// \param[out] intersections Empty list in which the intersections are placed.
inline void intersect(const Spiral& spiral_1, const SpiralSegment& spiral_2,
                      std::vector<PolarPoint>& intersections) {
	std::vector<PolarPoint> spiralIntersections;
	intersect(spiral_1, spiral_2.supportingSpiral(), spiralIntersections);
	for (const PolarPoint& p : spiralIntersections) {
		if (detail::checkIntersection(spiral_2, p)) {
			intersections.push_back(p);
		}
	}
}
/// \copydoc intersect(const Spiral&, const SpiralSegment&, std::vector<PolarPoint>&)
inline void intersect(const SpiralSegment& spiral_1, const Spiral& spiral_2,
                      std::vector<PolarPoint>& intersections) {
	intersect(spiral_2, spiral_1, intersections);
}
/// Computes the intersection (if any) between the given spiral segments.
/// \param[out] intersections Empty list in which the intersections are placed.
inline void intersect(const SpiralSegment& spiral_1, const SpiralSegment& spiral_2,
                      std::vector<PolarPoint>& intersections) {
	std::vector<PolarPoint> spiralIntersections;
	intersect(spiral_1.supportingSpiral(), spiral_2.supportingSpiral(), spiralIntersections);
	for (const PolarPoint& p : spiralIntersections) {
		if (detail::checkIntersection(spiral_1, p) && detail::checkIntersection(spiral_2, p)) {
			intersections.push_back(p);
		}
	}
}

// LINE/SPIRAL

/// Computes (at most two) intersections between the given line and the given
/// spiral.
///
/// In general a line and a spiral have infinitely many intersections. This
/// method produces the two intersections closest to the anchor of the spiral on
/// opposite sides.
///
/// \param[out] intersections Empty list in which the intersections are placed.
void intersect(const PolarLine& line, const Spiral& spiral, std::vector<PolarPoint>& intersections);
/// \copydoc intersect(const PolarLine&, const Spiral&, std::vector<PolarPoint>&)
inline void intersect(const Spiral& spiral, const PolarLine& line,
                      std::vector<PolarPoint>& intersections) {
	intersect(line, spiral, intersections);
}
/// Computes the intersection (if any) between the given line and the given
/// spiral segment.
/// \param[out] intersections Empty list in which the intersections are placed.
inline void intersect(const PolarLine& line, const SpiralSegment& spiral,
                      std::vector<PolarPoint>& intersections) {
	std::vector<PolarPoint> spiralIntersections;
	intersect(line, spiral.supportingSpiral(), spiralIntersections);
	for (const PolarPoint& p : spiralIntersections) {
		if (detail::checkIntersection(spiral, p)) {
			intersections.push_back(p);
		}
	}
}
/// \copydoc intersect(const PolarLine&, const SpiralSegment&, std::vector<PolarPoint>&)
inline void intersect(const SpiralSegment& spiral, const PolarLine& line,
                      std::vector<PolarPoint>& intersections) {
	intersect(line, spiral, intersections);
}
/// Computes the intersection (if any) between the given segment and the given
/// spiral.
/// \param[out] intersections Empty list in which the intersections are placed.
inline void intersect(const PolarSegment& line, const Spiral& spiral,
                      std::vector<PolarPoint>& intersections) {
	std::vector<PolarPoint> lineIntersections;
	intersect(line.supportingLine(), spiral, lineIntersections);
	for (const PolarPoint& p : lineIntersections) {
		if (detail::checkIntersection(line, p)) {
			intersections.push_back(p);
		}
	}
}
/// \copydoc intersect(const PolarSegment&, const Spiral&, std::vector<PolarPoint>&)
inline void intersect(const Spiral& spiral, const PolarSegment& line,
                      std::vector<PolarPoint>& intersections) {
	intersect(line, spiral, intersections);
}
/// Computes the intersection (if any) between the given segment and the given
/// spiral segment.
/// \param[out] intersections Empty list in which the intersections are placed.
inline void intersect(const PolarSegment& line, const SpiralSegment& spiral,
                      std::vector<PolarPoint>& intersections) {
	std::vector<PolarPoint> allIntersections;
	intersect(line.supportingLine(), spiral.supportingSpiral(), allIntersections);
	for (const PolarPoint& p : allIntersections) {
		if (detail::checkIntersection(line, p) && detail::checkIntersection(spiral, p)) {
			intersections.push_back(p);
		}
	}
}
/// \copydoc intersect(const PolarSegment&, const SpiralSegment&, std::vector<PolarPoint>&)
inline void intersect(const SpiralSegment& spiral, const PolarSegment& line,
                      std::vector<PolarPoint>& intersections) {
	intersect(line, spiral, intersections);
}

} // namespace cartocrow::flow_map

#endif //CARTOCROW_CORE_INTERSECTIONS_H
