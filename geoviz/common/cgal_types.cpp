/*
The GeoViz library implements algorithmic geo-visualization methods,
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 07-11-2019
*/

#include "cgal_types.h"

/**@file
 * Definitions of the basic types used throughout the library.
 *
 * This mostly concerns 2-dimensional geometric element types.
 *
 * All coordinates are in the coordinate system of the desired output, meaning that any coordinate system conversions must be done before presenting the input to the algorithms.
 *
 * When using the GeoViz website, the output may be shown in Leaflet.js on top of a base map. In this case, the coordinates should be in the CRS used by the base map, which is EPSG3857 for the default base map, i.e. OpenStreetMap.
 */

namespace geoviz
{

/**@typedef Kernel
 * @brief The geometry traits used throughout the library.
 */

/**@typedef Number
 * @brief The main number type.
 */

/**@typedef Point
 * @brief 2D fixed point location.
 */

/**@typedef Vector
 * @brief 2D vector, usually describing a displacement.
 */

/**@typedef Circle
 * @brief 2D circle.
 */

/**@typedef Polygon
 * @brief 2D straight-line polygon.
 */

} // namespace geoviz
