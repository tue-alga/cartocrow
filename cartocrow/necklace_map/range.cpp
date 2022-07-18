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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 09-12-2019
*/

#include "range.h"

namespace cartocrow::necklace_map {

Range::Range(const Number<Inexact>& from, const Number<Inexact>& to) : m_a(from), m_b(to) {}

Range::Range(const Range& range) : m_a(range.from()), m_b(range.to()) {}

const Number<Inexact>& Range::from() const {
	return m_a;
}

Number<Inexact>& Range::from() {
	return m_a;
}

const Number<Inexact>& Range::to() const {
	return m_b;
}

Number<Inexact>& Range::to() {
	return m_b;
}

bool Range::isValid() const {
	return from() <= to();
}

bool Range::isDegenerate() const {
	return from() == to();
}

bool Range::contains(const Number<Inexact>& value) const {
	return from() <= value && value <= to();
}

bool Range::containsInterior(const Number<Inexact>& value) const {
	return from() < value && value < to();
}

bool Range::intersects(const Range& range) const {
	return contains(range.from()) || range.contains(from());
}

bool Range::intersectsInterior(const Range& range) const {
	return (contains(range.from()) && range.from() != to()) ||
	       (range.contains(from()) && from() != range.to());
}

Number<Inexact> Range::length() const {
	return to() - from();
}

} // namespace cartocrow::necklace_map
