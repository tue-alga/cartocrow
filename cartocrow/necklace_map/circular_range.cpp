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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 06-05-2020
*/

#include "circular_range.h"

namespace cartocrow::necklace_map {

CircularRange::CircularRange(const Number<Inexact>& from_angle, const Number<Inexact>& to_angle)
    : Range(from_angle, to_angle) {
	if (M_2xPI <= to_angle - from_angle) {
		from() = 0;
		to() = M_2xPI;
	} else {
		from() = wrapAngle(from_angle);
		to() = wrapAngle(to_angle, from());
	}
}

CircularRange::CircularRange(const Range& range) : CircularRange(range.from(), range.to()) {}

bool CircularRange::isValid() const {
	if (isFull()) {
		return true;
	}
	return 0 <= from() && from() < M_2xPI && from() <= to() && to() < from() + M_2xPI;
}

bool CircularRange::isFull() const {
	return from() == 0 && to() == M_2xPI;
}

bool CircularRange::contains(const Number<Inexact>& value) const {
	const Number<Inexact> value_mod = wrapAngle(value, from());
	return from() <= value_mod && value_mod <= to();
}

bool CircularRange::containsInterior(const Number<Inexact>& value) const {
	const Number<Inexact> value_mod = wrapAngle(value, from());
	return from() < value_mod && value_mod < to();
}

bool CircularRange::intersects(const Range& range) const {
	CircularRange interval(range);
	return contains(interval.from()) || interval.contains(from());
}

bool CircularRange::intersectsInterior(const Range& range) const {
	CircularRange interval(range);
	return (contains(interval.from()) && wrapAngle(interval.from(), from()) != to()) ||
	       (interval.contains(from()) && wrapAngle(from(), interval.from()) != interval.to());
}

Number<Inexact> CircularRange::midpoint() const {
	return wrapAngle(.5 * (from() + to()));
}

} // namespace cartocrow::necklace_map
