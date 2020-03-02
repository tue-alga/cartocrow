/*
The Necklace Map library implements the algorithmic geo-visualization
method by the same name, developed by Bettina Speckmann and Kevin Verbeek
at TU Eindhoven (DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
Copyright (C) 2019  Netherlands eScience Center and TU Eindhoven

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


namespace geoviz
{
namespace necklace_map
{

/**@class CircleNecklace
 * @brief A full circle necklace.
 */

/**@brief Construct a circle necklace.
 *
 * The necklace kernel is the circle center.
 *
 * @param shape the circle covered by the necklace.
 */
CircleNecklace::CircleNecklace(const Circle& shape) : NecklaceShape(), shape_(shape)
{
  radius_ = CGAL::sqrt(shape_.squared_radius());
  length_ = M_2xPI * radius_;
}

const Point& CircleNecklace::kernel() const
{
  return shape_.center();
}

bool CircleNecklace::IsValid() const
{
  return 0 < radius_;
}

Box CircleNecklace::ComputeBoundingBox() const
{
  return shape_.bbox();
}

/*Number CircleNecklace::ComputeLength() const
{
  return length_;
}*/

Number CircleNecklace::ComputeRadius() const
{
  return radius_;
}

bool CircleNecklace::IntersectRay(const Number& angle_rad, Point& intersection) const
{
  const Vector relative = CGAL::sqrt(shape_.squared_radius()) * Vector( std::cos(angle_rad), std::sin(angle_rad) );
  intersection = kernel() + relative;
  return true;
}

void CircleNecklace::Accept(NecklaceShapeVisitor& visitor)
{
  visitor.Visit(*this);
}

} // namespace necklace_map
} // namespace geoviz
