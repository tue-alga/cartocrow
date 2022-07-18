/*
The Necklace Map library implements the algorithmic geo-visualization
method by the same name, developed by Bettina Speckmann and Kevin Verbeek
at TU Eindhoven (DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 15-01-2020
*/

#ifndef CARTOCROW_NECKLACE_MAP_NECKLACE_SHAPE_H
#define CARTOCROW_NECKLACE_MAP_NECKLACE_SHAPE_H

#include <memory>

#include "../core/core.h"
#include "circular_range.h"

namespace cartocrow::necklace_map {

class CircleNecklace;
class BezierNecklace;

class NecklaceShapeVisitor {
  public:
	virtual void Visit(CircleNecklace& shape);
	virtual void Visit(BezierNecklace& shape);
}; // class NecklaceShapeVisitor

class NecklaceShape {
  public:
	virtual ~NecklaceShape() = default;

	using Ptr = std::shared_ptr<NecklaceShape>;

	virtual const Point<Inexact>& kernel() const = 0;

	virtual bool isValid() const = 0;

	virtual bool intersectRay(const Number<Inexact>& angle_rad,
	                          Point<Inexact>& intersection) const = 0;

	virtual Box computeBoundingBox() const = 0;

	virtual Number<Inexact> computeCoveringRadiusRad(const Range& range,
	                                                 const Number<Inexact>& radius) const = 0;

	virtual Number<Inexact> computeDistanceToKernel(const Range& range) const = 0;

	Number<Inexact> computeAngleRad(const Point<Inexact>& point) const;

	virtual Number<Inexact> computeAngleAtDistanceRad(const Number<Inexact>& angle_rad,
	                                                  const Number<Inexact>& distance) const = 0;

	virtual void accept(NecklaceShapeVisitor& visitor) = 0;
	// TODO(tvl) should the necklace contain methods for adapting the 1D solution to the 2D solution?
	// This would mainly come into play for Bezier necklaces. For impl: check the 30x for loop in the Java prototype code.
	// Another case would be when a bead contains the necklace center;
	// in this case, the scale factor should be reduced such that the bead does not contain the necklace center
	// (could this cause recursive failure and if so, in which cases? This *may* actually prove scientifically interesting).
};

} // namespace cartocrow::necklace_map

#endif //CARTOCROW_NECKLACE_MAP_NECKLACE_SHAPE_H
