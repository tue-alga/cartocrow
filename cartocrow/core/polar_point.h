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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 04-09-2020
*/

#ifndef CARTOCROW_CORE_POLAR_POINT_H
#define CARTOCROW_CORE_POLAR_POINT_H

#include <cmath>
#include <ostream>

#include "cartocrow/core/cgal_types.h"

namespace cartocrow {

class PolarPoint {
  public:
	PolarPoint();

	PolarPoint(const CGAL::Origin& o);

	template <typename T1, typename T2> PolarPoint(const T1& R, const T2& phi);

	PolarPoint(const PolarPoint& p);

	PolarPoint(const PolarPoint& p, const Vector& t);

	explicit PolarPoint(const Point& p);

	PolarPoint(const Point& p, const Vector& t);

	const Number& R() const;
	const Number& phi() const;

	Point to_cartesian() const;

  private:
	static PolarPoint to_polar(const Point& p);
	static PolarPoint translate_pole(const PolarPoint& p, const Vector& t);

	Number R_, phi_;
}; // class PolarPoint

bool operator==(const PolarPoint& p, const PolarPoint& q);
bool operator!=(const PolarPoint& p, const PolarPoint& q);

std::ostream& operator<<(std::ostream& os, const PolarPoint& point);

} // namespace cartocrow

#include "polar_point.inc"

#endif //CARTOCROW_CORE_POLAR_POINT_H
