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

#ifndef CARTOCROW_CORE_SPIRAL_SEGMENT_H
#define CARTOCROW_CORE_SPIRAL_SEGMENT_H

#include <ostream>

#include "cartocrow/core/core_types.h"
#include "cartocrow/core/polar_point.h"
#include "cartocrow/core/spiral.h"

namespace cartocrow {

class SpiralSegment : public Spiral {
  public:
	SpiralSegment(const PolarPoint& point_1, const PolarPoint& point_2);

	SpiralSegment(const PolarPoint& far, const Number& angle_rad, const Number& R_min);

	SpiralSegment(const PolarPoint& anchor, const Number& angle_rad, const Number& R_min,
	              const Number& R_max);

	const PolarPoint far() const;

	const PolarPoint near() const;

	const Number& R_min() const;

	const Number& R_max() const;

	bool ContainsT(const Number& t) const;

	bool ContainsR(const Number& R) const;

  private:
	Number R_min_, R_max_;
}; // class SpiralSegment

std::ostream& operator<<(std::ostream& os, const SpiralSegment& segment);

} // namespace cartocrow

#endif //CARTOCROW_CORE_SPIRAL_SEGMENT_H
