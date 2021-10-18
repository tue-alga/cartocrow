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

/// A description of a drawing using a \ref GeometryRenderer.
/**
 * CartoCrow's rendering system is built around *paintings* and *renderers*. A
 * renderer is an implementation of a set of rendering methods for a certain
 * target, such as real-time rendering to a Qt panel (\ref GeometryWidget), or
 * writing to an Ipe file (\ref IpeRenderer). All available renderers descend
 * from the abstract class \ref GeometryRenderer, which provides a set of
 * standard rendering methods, such that the API for rendering to each target is
 * identical.
 * 
 * A painting is simply a list of calls to rendering methods (provided by the
 * GeometryRenderer) that make up a drawing. This is implemented in a
 * straightforward manner by the \ref paint() method, which gets the \ref
 * GeometryRenderer as its argument. So, to render some object with the
 * rendering system, create a new subclass of \ref GeometryPainting, implement
 * its \ref paint() method, and finally construct a specific renderer with this
 * painting.
 * 
 * Many objects in CartoCrow already provide a painting. For example, a necklace
 * map can be rendered by using the \ref cartocrow::necklace_map::Painting. To
 * add additional objects to an existing painting (for example, to add a legend
 * to a necklace map), make a new painting, call the necklace painting's \ref
 * paint() method from it, and then draw the additional objects.
 */
class GeometryPainting {

  public:
    /// Performs the rendering for this painting.
	/**
	 * This is a pure virtual method; implement it define what should be
	 * rendered by the painting.
	 */
	virtual void paint(GeometryRenderer& renderer) = 0;
};

} // namespace renderer
} // namespace cartocrow

#endif //CARTOCROW_GEOMETRY_PAINTING
