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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 25-02-2020
*/

#ifndef CARTOCROW_NECKLACE_MAP_CIRCLE_NECKLACE_H
#define CARTOCROW_NECKLACE_MAP_CIRCLE_NECKLACE_H

#include "../core/core.h"
#include "necklace_shape.h"

namespace cartocrow {
namespace necklace_map {

class CircleNecklace : public NecklaceShape {
  public:
	using Ptr = std::shared_ptr<CircleNecklace>;

	explicit CircleNecklace(const Circle& shape);

	const Point& kernel() const override;

	const Number& draw_bounds_cw_rad() const;
	Number& draw_bounds_cw_rad();

	const Number& draw_bounds_ccw_rad() const;
	Number& draw_bounds_ccw_rad();

	bool IsValid() const override;

	/*bool IsEmpty() const override;

  bool IsClosed() const override;*/

	bool IntersectRay(const Number& angle_rad, Point& intersection) const override;

	Box ComputeBoundingBox() const override;

	Number ComputeRadius() const;

	Number ComputeCoveringRadiusRad(const Range::Ptr& range, const Number& radius) const override;

	Number ComputeDistanceToKernel(const Range::Ptr& range) const override;

	Number ComputeAngleAtDistanceRad(const Number& angle_rad, const Number& distance) const override;

	virtual void Accept(NecklaceShapeVisitor& visitor) override;

	Circle shape_;

  private:
	Number radius_;

	Number draw_bounds_cw_rad_, draw_bounds_ccw_rad_;
}; // class CircleNecklace

} // namespace necklace_map
} // namespace cartocrow

#endif //CARTOCROW_NECKLACE_MAP_CIRCLE_NECKLACE_H
