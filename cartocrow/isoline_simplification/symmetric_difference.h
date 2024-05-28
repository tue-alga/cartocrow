/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
Copyright (C) 2024 TU Eindhoven

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
*/

#ifndef CARTOCROW_SYMMETRIC_DIFFERENCE_H
#define CARTOCROW_SYMMETRIC_DIFFERENCE_H

#include "types.h"
#include "isoline.h"

namespace cartocrow::isoline_simplification {
double symmetric_difference(const Isoline<K>& original, const Isoline<K>& simplified);
}
#endif //CARTOCROW_SYMMETRIC_DIFFERENCE_H
