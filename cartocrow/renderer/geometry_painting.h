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
*/

#ifndef CARTOCROW_GEOMETRY_PAINTING
#define CARTOCROW_GEOMETRY_PAINTING

#include "geometry_renderer.h"

namespace cartocrow {
namespace renderer {

/**
 * A GeometryPainting specifies what you want to draw using a \link
 * GeometryRenderer.
 *
 * To use this, override the \link paint() method, using the methods provided
 * by the \link GeometryRenderer. Then, construct a specific renderer (e.g.,
 * a \link GeometryWidget) with this painting.
 */
class GeometryPainting {

  public:
	virtual void paint(GeometryRenderer& renderer) = 0;
};

} // namespace renderer
} // namespace cartocrow

#endif //CARTOCROW_GEOMETRY_PAINTING
