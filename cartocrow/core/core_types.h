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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 05-12-2019
*/

#ifndef CARTOCROW_CORE_CORE_TYPES_H
#define CARTOCROW_CORE_CORE_TYPES_H

#include <cmath>

#include "cgal_types.h"
#include "polygon.h"

namespace cartocrow {

/// An RGB color. Used for storing the color of elements to be drawn.
struct Color {
	/// Red component (integer 0-255).
	int r;
	/// Green component (integer 0-255).
	int g;
	/// Blue component (integer 0-255).
	int b;
};

} // namespace cartocrow

#endif //CARTOCROW_CORE_CORE_TYPES_H
