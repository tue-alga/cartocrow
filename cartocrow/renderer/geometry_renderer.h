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

#ifndef CARTOCROW_RENDERER_GEOMETRY_RENDERER_H
#define CARTOCROW_RENDERER_GEOMETRY_RENDERER_H

#include "../core/bezier.h"
#include "../core/core.h"
#include "../core/region_map.h"
#include "../core/polyline.h"

namespace cartocrow::renderer {

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
	virtual void draw(const Point<Inexact>& p) = 0;
	/// Draws a single line segment with the currently set style.
	virtual void draw(const Segment<Inexact>& s) = 0;
	/// Draws a simple polygon with the currently set style.
	virtual void draw(const Polygon<Inexact>& p) = 0;
	/// Draws a polygon with holes with the currently set style.
	virtual void draw(const PolygonWithHoles<Inexact>& p) = 0;
	/// Draws a circle with the currently set style.
	virtual void draw(const Circle<Inexact>& c) = 0;
	/// Draws a Bézier spline with the currently set style.
	void draw(const BezierCurve& c);
	/// Draws a Bézier spline with the currently set style.
	virtual void draw(const BezierSpline& s) = 0;
	/// Draws a polygon set with the currently set style.
	virtual void draw(const PolygonSet<Inexact>& p);
	/// Draws a line with the currently set style.
	virtual void draw(const Line<Inexact>& l) = 0;
	/// Draws a ray with the currently set style.
	virtual void draw(const Ray<Inexact>& r) = 0;
	/// Draws a polyline with the currently set style.
	virtual void draw(const Polyline<Inexact>& p) = 0;

	/// Draws an exact geometry with the currently set style by approximating it.
	template<class ExactGeometry>
	void draw(const ExactGeometry& g) {
		draw(approximate(g));
	};

	/// Draws a string at a given location.
	/// The string is drawn centered horizontally around the location given.
	virtual void drawText(const Point<Inexact>& p, const std::string& text) = 0;
	/// Draws a string at a given location.
	/// The string is drawn centered horizontally around the location given.
	template <class K>
	void drawText(const Point<K>& p, const std::string& text) {
		drawText(approximate(p), text);
	}

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
	/// Sets the fill opacity of the renderer (range 0-255).
	virtual void setFillOpacity(int alpha) = 0;

	/// @}
};

} // namespace cartocrow::renderer

#endif //CARTOCROW_RENDERER_GEOMETRY_RENDERER_H
