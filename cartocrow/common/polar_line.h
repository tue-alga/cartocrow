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

#ifndef CARTOCROW_COMMON_POLAR_LINE_H
#define CARTOCROW_COMMON_POLAR_LINE_H

#include <ostream>

#include "cartocrow/common/core_types.h"
#include "cartocrow/common/polar_point.h"

namespace cartocrow {

class PolarLine {
  public:
	explicit PolarLine(const PolarPoint& closest);

	PolarLine(const PolarPoint& point_1, const PolarPoint& point_2);

	const PolarPoint& foot() const;

	PolarPoint& foot();

	bool ContainsR(const Number& R) const;

	bool ContainsPhi(const Number& phi) const;

	Number EvaluateR(const Number& t) const;

	Number EvaluatePhi(const Number& t) const;

	PolarPoint Evaluate(const Number& t) const;

	Number ComputeT(const Number& phi) const;

	template <class OutputIterator> int CollectT(const Number& R, OutputIterator t) const;

	Number ComputeR(const Number& phi) const;

	template <class OutputIterator> int CollectPhi(const Number& R, OutputIterator phi) const;

	bool ComputeAngle(const Number& R, Number& angle_rad) const;

  protected:
	PolarLine() {}

	// Returns the signed distance from point 1 to point 2.
	Number SetFoot(const PolarPoint& point_1, const PolarPoint& point_2);

  private:
	PolarPoint foot_;
}; // class PolarLine

std::ostream& operator<<(std::ostream& os, const PolarLine& line);

} //namespace cartocrow

#include "polar_line.inc"

#endif //CARTOCROW_COMMON_POLAR_LINE_H
