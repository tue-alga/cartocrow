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

#include "circle_necklace.h"

namespace cartocrow {
namespace necklace_map {

/**@class CircleNecklace
 * @brief A full circle necklace.
 */

/**@fn CircleNecklace::Ptr
 * @brief The preferred pointer type for storing or sharing a circle necklace.
 */

/**@brief Construct a circle necklace.
 *
 * The necklace kernel is the circle center.
 * @param shape the circle covered by the necklace.
 */
CircleNecklace::CircleNecklace(const Circle<Inexact>& shape)
    : NecklaceShape(), shape_(shape), draw_bounds_cw_rad_(0), draw_bounds_ccw_rad_(0) {
	radius_ = CGAL::sqrt(shape_.squared_radius());
}

const Point<Inexact>& CircleNecklace::kernel() const {
	return shape_.center();
}

/**@brief Access the clockwise endpoint for drawing the necklace.
 *
 * If this is equal to the counterclockwise endpoint, the full circle is drawn.
 * @return the clockwise endpoint for drawing the necklace.
 */
const Number<Inexact>& CircleNecklace::draw_bounds_cw_rad() const {
	return draw_bounds_cw_rad_;
}

/**@brief Access the clockwise endpoint for drawing the necklace.
 *
 * If this is equal to the counterclockwise endpoint, the full circle is drawn.
 * @return the clockwise endpoint for drawing the necklace.
 */
Number<Inexact>& CircleNecklace::draw_bounds_cw_rad() {
	return draw_bounds_cw_rad_;
}

/**@brief Access the counterclockwise endpoint for drawing the necklace.
 *
 * If this is equal to the clockwise endpoint, the full circle is drawn.
 * @return the counterclockwise endpoint for drawing the necklace.
 */
const Number<Inexact>& CircleNecklace::draw_bounds_ccw_rad() const {
	return draw_bounds_ccw_rad_;
}

/**@brief Access the counterclockwise endpoint for drawing the necklace.
 *
 * If this is equal to the clockwise endpoint, the full circle is drawn.
 * @return the counterclockwise endpoint for drawing the necklace.
 */
Number<Inexact>& CircleNecklace::draw_bounds_ccw_rad() {
	return draw_bounds_ccw_rad_;
}

bool CircleNecklace::isValid() const {
	return 0 < radius_;
}

/*bool CircleNecklace::IsEmpty() const
{
  return 0 < radius_;
}

bool CircleNecklace::IsClosed() const
{
  return true;
}*/

bool CircleNecklace::intersectRay(const Number<Inexact>& angle_rad,
                                  Point<Inexact>& intersection) const {
	const Vector<Inexact> relative = CGAL::sqrt(shape_.squared_radius()) *
	                                 Vector<Inexact>(std::cos(angle_rad), std::sin(angle_rad));
	intersection = kernel() + relative;
	return true;
}

Box CircleNecklace::computeBoundingBox() const {
	return shape_.bbox();
}

/**@brief Compute the radius of the circle covered by this necklace.
 * @return the radius.
 */
Number<Inexact> CircleNecklace::computeRadius() const {
	return radius_;
}

Number<Inexact> CircleNecklace::computeCoveringRadiusRad(const Range& range,
                                                         const Number<Inexact>& radius) const {
	return std::asin(radius / radius_);
}

Number<Inexact> CircleNecklace::computeDistanceToKernel(const Range& range) const {
	return radius_;
}

Number<Inexact> CircleNecklace::computeAngleAtDistanceRad(const Number<Inexact>& angle_rad,
                                                          const Number<Inexact>& distance) const {
	const Number<Inexact> distance_abs = std::abs(distance);
	assert(distance_abs <= 2 * radius_);
	if (distance_abs == 2 * radius_) {
		return angle_rad + M_PI;
	}

	const Number<Inexact> angle_diff = 2 * std::asin(distance_abs / (2 * radius_));
	return 0 < distance ? angle_rad + angle_diff : angle_rad - angle_diff;
}

void CircleNecklace::accept(NecklaceShapeVisitor& visitor) {
	visitor.Visit(*this);
}

} // namespace necklace_map
} // namespace cartocrow
