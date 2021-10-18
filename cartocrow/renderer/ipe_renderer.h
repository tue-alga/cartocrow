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

#ifndef CARTOCROW_IPE_RENDERER
#define CARTOCROW_IPE_RENDERER

#include <ipeattributes.h>
#include <ipelib.h>

#include <filesystem>
#include <stack>

#include "geometry_painting.h"
#include "geometry_renderer.h"

namespace cartocrow {
namespace renderer {

/// The style for the Ipe renderer.
struct IpeRendererStyle {
	/// The draw mode.
	int m_mode = GeometryRenderer::stroke | GeometryRenderer::fill;
	/// The diameter of points.
	double m_pointSize = 10;
	/// The color of points and lines.
	ipe::Color m_strokeColor = ipe::Color(0, 0, 0);
	/// The width of lines.
	double m_strokeWidth = 1;
	/// The color of filled shapes.
	ipe::Color m_fillColor = ipe::Color(0, 102, 203);
	/// The opacity of filled shapes, as a symbolic Ipe attribute.
	ipe::Attribute m_fillOpacity;
};

/// Ipe specialization of the GeometryRenderer.
/**
 * Construct the IpeRenderer with a painting, and call \ref save() to render
 * the painting to a file. \ref save() can be called more than one time (for
 * example after changing the painting) if desired.
 *
 * Ipelib works with raw pointers a lot. In general, after adding an object to
 * a parent (for example, adding a page to a document), the parent takes
 * possession of the added object. This means that the parent's destructor will
 * also delete the added object. Because of this behavior, IpeRenderer
 * unavoidably has several methods that return raw pointers to newly allocated
 * objects.
 */
class IpeRenderer : public GeometryRenderer {

  public:
	/// Constructs a IpeRenderer for the given painting.
	IpeRenderer(GeometryPainting& painting);

	/// Saves the painting to an Ipe file with the given name.
	void save(const std::filesystem::path& file);

	void draw(const Point& p) override;
	void draw(const Segment& s) override;
	void draw(const Polygon& p) override;
	void draw(const Polygon_with_holes& p) override;
	void draw(const Circle& c) override;
	void draw(const Box& b) override;
	void draw(const BezierSpline& s) override;
	void drawText(const Point& p, const std::string& text) override;

	void pushStyle() override;
	void popStyle() override;
	void setMode(int mode) override;
	void setStroke(Color color, double width) override;
	void setFill(Color color) override;
	void setFillOpacity(int alpha) override;

  private:
	/// Converts a polygon to an Ipe curve.
	ipe::Curve* convertPolygonToCurve(const Polygon& p) const;
	/// Returns Ipe attributes to style an Ipe path with the current style.
	ipe::AllAttributes getAttributesForStyle() const;

	/// The painting we're drawing.
	GeometryPainting& m_painting;
	/// The current drawing style.
	IpeRendererStyle m_style;
	/// A stack of drawing styles, used by \ref pushStyle() and \ref popStyle()
	/// to store previously pushed styles.
	std::stack<IpeRendererStyle> m_styleStack;
	/// The page we are drawing to. Only valid while drawing (in \ref save()).
	/**
	 * Ipelib deletes pages when the Document is destructed, so we need to
     * avoid destructing pages ourselves.
     */
	ipe::Page* m_page;
	/// Ipe style sheet with the alpha values in this drawing.
	ipe::StyleSheet* m_alphaSheet;
};

} // namespace renderer
} // namespace cartocrow

#endif //CARTOCROW_IPE_RENDERER
