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

#ifndef CARTOCROW_CORE_POLAR_LINE_H
#define CARTOCROW_CORE_POLAR_LINE_H

#include <ostream>

#include "../core/core.h"
#include "polar_point.h"

namespace cartocrow::necklace_map {

class PolarLine {
  public:
	explicit PolarLine(const PolarPoint& closest);

	PolarLine(const PolarPoint& point_1, const PolarPoint& point_2);

	const PolarPoint& foot() const;

	PolarPoint& foot();

	bool ContainsR(const Number<Inexact>& R) const;

	bool ContainsPhi(const Number<Inexact>& phi) const;

	Number<Inexact> EvaluateR(const Number<Inexact>& t) const;

	Number<Inexact> EvaluatePhi(const Number<Inexact>& t) const;

	PolarPoint Evaluate(const Number<Inexact>& t) const;

	Number<Inexact> ComputeT(const Number<Inexact>& phi) const;

	template <class OutputIterator> int CollectT(const Number<Inexact>& R, OutputIterator t) const;

	Number<Inexact> ComputeR(const Number<Inexact>& phi) const;

	template <class OutputIterator>
	int CollectPhi(const Number<Inexact>& R, OutputIterator phi) const;

	bool ComputeAngle(const Number<Inexact>& R, Number<Inexact>& angle_rad) const;

  protected:
	PolarLine() {}

	// Returns the signed distance from point 1 to point 2.
	Number<Inexact> SetFoot(const PolarPoint& point_1, const PolarPoint& point_2);

  private:
	PolarPoint foot_;
}; // class PolarLine

std::ostream& operator<<(std::ostream& os, const PolarLine& line);

} // namespace cartocrow::necklace_map

#include "polar_line.inc"

#endif //CARTOCROW_CORE_POLAR_LINE_H
