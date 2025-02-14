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
#include "../core/polyline.h"
#include "../core/halfplane.h"
#include "render_path.h"

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

	enum LineCap {
		ButtCap,
		RoundCap,
		SquareCap,
	};

	enum LineJoin {
		RoundJoin,
		BevelJoin,
		MiterJoin,
	};

	enum HorizontalTextAlignment {
		AlignLeft,
		AlignRight,
		AlignHCenter,
	};

	enum VerticalTextAlignment {
		AlignTop,
		AlignBottom,
		AlignVCenter,
		AlignBaseline,
	};

	/// \name Drawing methods
	/// @{

	/// Draws a single point with the currently set style.
	virtual void draw(const Point<Inexact>& p) = 0;
	/// Draws a single line segment with the currently set style.
	void draw(const Segment<Inexact>& s);
	/// Draws a rectangle
	void draw(const Rectangle<Inexact>& r);
	/// Draws a triangle
	void draw(const Triangle<Inexact>& t);
	/// Draws a box
	void draw(const Box& r);
	/// Draws a simple polygon with the currently set style.
	void draw(const Polygon<Inexact>& p);
	/// Draws a polyline with the currently set style.
	void draw(const Polyline<Inexact>& p);
	/// Draws a polygon with holes with the currently set style.
	void draw(const PolygonWithHoles<Inexact>& p);
	/// Draws a polygon set with the currently set style.
	void draw(const PolygonSet<Inexact>& p);
	/// Draws a circle with the currently set style.
	virtual void draw(const Circle<Inexact>& c) = 0;
	/// Draws a Bézier curve with the currently set style.
	void draw(const BezierCurve& c);
	/// Draws a Bézier spline with the currently set style.
	virtual void draw(const BezierSpline& s) = 0;
	/// Draws a line with the currently set style.
	virtual void draw(const Line<Inexact>& l) = 0;
	/// Draws a ray with the currently set style.
	virtual void draw(const Ray<Inexact>& r) = 0;
	/// Draws a halfplane with the currently set style.
	virtual void draw(const Halfplane<Inexact>& h) = 0;
	/// Draws a \ref RenderPath with the currently set style.
	virtual void draw(const RenderPath& p) = 0;

	/// Draws an exact geometry with the currently set style by approximating it.
	template<class ExactGeometry>
	void draw(const ExactGeometry& g) {
		draw(approximate(g));
	};

	/// Draws a string at a given location.
	/// The string is aligned as specified.
	/// If \p escape is set to true, then the function escapes any characters that
	/// have special meaning in the renderer (characters like '\<', '\', '%').
	/// \sa setHorizontalTextAlignment
	/// \sa setVerticalTextAlignment
	virtual void drawText(const Point<Inexact>& p, const std::string& text, bool escape=true) = 0;
	/// Draws a string at a given location.
	/// The string is aligned as specified.
	/// If \p escape is set to true, then the function escapes any characters that
	/// have special meaning in the renderer (characters like '\<', '\', '%').
	/// \sa setHorizontalTextAlignment
	/// \sa setVerticalTextAlignment
	template <class K>
	void drawText(const Point<K>& p, const std::string& text, bool escape=true) {
		drawText(approximate(p), text, escape);
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
	///
	/// If `absoluteWidth` is `false`, in interactive renderers, the width is
	/// interpreted in screen coordinates, so when zooming in the stroke doesn't
	/// become thicker. If it is `true`, the width is interpreted in drawing
	/// coordinates.
	virtual void setStroke(Color color, double width, bool absoluteWidth = false) = 0;
	/// Sets the stroke opacity of the renderer (range 0-255).
	virtual void setStrokeOpacity(int alpha) = 0;
	/// Sets the fill color of the renderer.
	virtual void setFill(Color color) = 0;
	/// Sets the fill opacity of the renderer (range 0-255).
	virtual void setFillOpacity(int alpha) = 0;
    /// Sets clip path.
    virtual void setClipPath(const RenderPath& clipPath) = 0;
    /// Enable or disable clipping.
    virtual void setClipping(bool enable) = 0;
	/// Set line join.
	virtual void setLineJoin(LineJoin lineJoin) = 0;
	/// Set line cap.
	virtual void setLineCap(LineCap lineCap) = 0;
	/// Set horizontal text alignment.
	virtual void setHorizontalTextAlignment(HorizontalTextAlignment alignment) = 0;
	/// Set vertical text alignment.
	virtual void setVerticalTextAlignment(VerticalTextAlignment alignment) = 0;
	/// @}
};

} // namespace cartocrow::renderer

#endif //CARTOCROW_RENDERER_GEOMETRY_RENDERER_H
