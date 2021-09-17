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

#ifndef CARTOCROW_GEOMETRY_RENDERER
#define CARTOCROW_GEOMETRY_RENDERER

#include <cartocrow/common/cgal_types.h>
#include <cartocrow/common/polygon.h>
#include <cartocrow/common/region.h>

namespace cartocrow {
namespace renderer {

/// An interface for rendering geometric objects to a GUI or a file.
/**
 * GeometryRenderer is a shared interface for all targets CartoCrow is able to
 * render to (currently a Qt panel, an Ipe file, or an SVG file). Subclasses of
 * this class are provided for each specific render target. The intended use is
 * to use the painting methods in this class from a GeometryPainting.
 *
 *
 * ## Styling
 *
 * GeometryRenderer has a set of methods to change the style it renders objects
 * with. It remembers the style set for any subsequent drawing operations.
 * Initially, the style is set to sensible default values (black stroke and
 * fill, stroke width 1, etc.) The methods \ref pushStyle() and \ref popStyle()
 * store the current style onto a stack, and restore the style last stored to
 * the stack, respectively. These methods can be used to draw with a different
 * style temporarily.
 *
 *
 * ## Native painter access
 *
 * In case advanced drawing operations are not possible with the interface
 * provided by GeometryRenderer, access is provided to the underlying native
 * painters. Of course, the caveat is that code written for a specific painter
 * will not work for other render targets: `nullptr` is returned when trying to
 * access the native painter of a different render target.
 *
 * The suggested idiom to draw with native painters for the various render
 * targets is:
 *
 * ```
 *     if (QPainter* painter = renderer.getQPainter()) {
 *         // do Qt-specific painting with QPainter
 *     } else if (Page* ipePage = renderer.getIpePage()) {
 *         // do Ipe-specific painting by adding objects to its Page
 *     }
 *     // etc.
 * ```
 */
class GeometryRenderer {

  public:
	/// Defines how shapes are drawn.
	enum DrawMode {
		/// When drawing a non-linear shape, stroke its outline with the
		/// current stroke. (Linear features such as lines and curves are
		/// always stroked, regardless of whether this flag is set.)
		stroke = 1 << 0,
		/// When drawing a non-linear shape, stroke its inside with the current
		/// fill.
		fill = 1 << 1,
		/// When drawing a segment, polyline, or polygon, draw its vertices
		/// with the current point style.
		vertices = 1 << 2
	};

	/// \name Drawing methods
	/// @{

	/// Draws a single point with the currently set style.
	virtual void draw(const Point& p) = 0;
	/// Draws a single line segment with the currently set style.
	virtual void draw(const Segment& s) = 0;
	/// Draws a simple polygon with the currently set style.
	virtual void draw(const Polygon& p) = 0;
	/// Draws a polygon with holes with the currently set style.
	virtual void draw(const Polygon_with_holes& p) = 0;
	/// Draws a circle with the currently set style.
	virtual void draw(const Circle& p) = 0;
	/// Draws an axis-aligned bounding box with the currently set style.
	virtual void draw(const Box& p) = 0;
	/// Draws a map region with the currently set style.
	virtual void draw(const Region& r);

	/// @}

	/// \name Style settings
	/// @{

	/// Stores the current style (stroke style, fill style, etc.) of this
	/// renderer onto a stack, to be retrieved later by \ref popStyle().
	virtual void pushStyle() = 0;
	/// Restores a style stored previously by \ref pushStyle().
	virtual void popStyle() = 0;

	/// Sets the draw mode (whether shapes should be stroked, filled, etc.)
	virtual void setMode(int mode) = 0;
	/// Sets the stroke style of the renderer.
	virtual void setStroke(Color color, double width) = 0;
	/// Sets the fill color of the renderer.
	virtual void setFill(Color color) = 0;

	/// @}

	/// \name Native painter access
	/// @{

	/// Accesses the underlying QPainter, or returns an empty pointer if this
	/// GeometryRenderer does not target a QPainter.
	/**
	 * The caller is free to do with the QPainter what they wish, to do any
	 * custom rendering required. The QPainter does not need to be restored to
	 * its original state.
	 */
	/*virtual std::unique_ptr<QPainter> getQPainter() {
		return std::unique_ptr<QPainter>();
	}*/
	// TODO [ws] This method cannot work like this, because it would need
	// #include <QPainter>
	// and that would mean we need to link to Qt everywhere in CartoCrow,
	// which is ugly.

	/// @}
};

} // namespace renderer
} // namespace cartocrow

#endif //CARTOCROW_GEOMETRY_RENDERER
