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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 26-02-2021
*/

#ifndef GEOVIZ_COMMON_DETAIL_POLAR_BOUNDS_H
#define GEOVIZ_COMMON_DETAIL_POLAR_BOUNDS_H

#include "geoviz/common/core_types.h"
#include "geoviz/common/polar_segment.h"
#include "geoviz/common/spiral.h"
#include "geoviz/common/spiral_segment.h"


namespace geoviz
{

Box ConstructBoundingBox(const PolarSegment& segment);

Box ConstructBoundingBox(const Spiral& spiral);

Box ConstructBoundingBox(const SpiralSegment& segment);

} // namespace geoviz

#endif //GEOVIZ_COMMON_DETAIL_POLAR_BOUNDS_H
