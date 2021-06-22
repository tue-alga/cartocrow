/*
The GeoViz library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 05-12-2019
*/

#ifndef GEOVIZ_COMMON_CORE_TYPES_H
#define GEOVIZ_COMMON_CORE_TYPES_H

#include <cmath>

#include "geoviz/common/cgal_types.h"
#include "geoviz/common/polygon.h"

namespace geoviz
{

/// The value 2*PI, defined for convenience.
constexpr const Number M_2xPI = 2 * M_PI;

Number Modulo(const Number& value, const Number& start = 0, const Number& range = M_2xPI);

Number ModuloNonZero(const Number& value, const Number& start = 0, const Number& range = M_2xPI);

} // namespace geoviz

#endif //GEOVIZ_COMMON_CORE_TYPES_H
