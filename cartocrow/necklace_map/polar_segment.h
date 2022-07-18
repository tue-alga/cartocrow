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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 19-02-2021
*/

#ifndef CARTOCROW_CORE_POLAR_SEGMENT_H
#define CARTOCROW_CORE_POLAR_SEGMENT_H

#include <ostream>

#include "../core/core.h"
#include "circular_range.h"
#include "polar_line.h"
#include "polar_point.h"

namespace cartocrow::necklace_map {

class PolarSegment : public PolarLine {
  public:
	PolarSegment(const PolarPoint& point_1, const PolarPoint& point_2);

	Number<Inexact> FromT() const;

	Number<Inexact> ToT() const;

	Number<Inexact> R_min() const;

	Number<Inexact> R_max() const;

	bool IsLeft() const;
	bool IsRight() const;
	bool IsCollinear() const;

	bool ContainsFoot() const;

	bool ContainsT(const Number<Inexact>& t) const;

	// Note that the following methods shadow the methods of PolarLine, which are not virtual.

	bool ContainsR(const Number<Inexact>& R) const;

	bool ContainsPhi(const Number<Inexact>& phi) const;

	Number<Inexact> EvaluateR(const Number<Inexact>& t) const;

	Number<Inexact> EvaluatePhi(const Number<Inexact>& t) const;

	PolarPoint Evaluate(const Number<Inexact>& t) const;

	Number<Inexact> ComputeT(const Number<Inexact>& phi) const;

	template <class OutputIterator> int CollectT(const Number<Inexact>& R, OutputIterator t) const;

	template <class OutputIterator>
	int CollectPhi(const Number<Inexact>& R, OutputIterator phi) const;

	PolarPoint ComputeClosestToPole() const;

	const PolarLine& SupportingLine() const;

  private:
	// We will call the 't' value of the base PolarLine the 'distance'.
	Number<Inexact> ToDistance(const Number<Inexact>& t) const;
	Number<Inexact> ToT(const Number<Inexact>& distance) const;

	Number<Inexact> offset_, multiplier_;
}; // class PolarSegment

std::ostream& operator<<(std::ostream& os, const PolarSegment& line);

} // namespace cartocrow::necklace_map

#include "polar_segment.inc"

#endif //CARTOCROW_CORE_POLAR_SEGMENT_H
