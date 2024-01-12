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

#ifndef CARTOCROW_RENDERER_IPE_RENDERER_H
#define CARTOCROW_RENDERER_IPE_RENDERER_H

#include <ipeattributes.h>
#include <ipelib.h>

#include <filesystem>
#include <stack>

#include "geometry_painting.h"
#include "geometry_renderer.h"

namespace cartocrow::renderer {

/// The style for the Ipe renderer.
struct IpeRendererStyle {
	/// The draw mode.
	int m_mode = GeometryRenderer::stroke;
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
 * ## Ipelib
 *
 * Ipelib works with raw pointers a lot. In general, after adding an object to
 * a parent (for example, adding a page to a document), the parent takes
 * possession of the added object. This means that the parent's destructor will
 * also delete the added object. Because of this behavior, IpeRenderer
 * unavoidably has several methods that return raw pointers to newly allocated
 * objects.
 *
 * ## Rendering strings
 *
 * Ipe uses LaTeX to render text, so any strings rendered through the
 * IpeRenderer will be interpreted by LaTeX. IpeRenderer escapes strings
 * containing reserved characters to make sure they get displayed correctly.
 */
class IpeRenderer : public GeometryRenderer {

  public:
	IpeRenderer() = default;

	/// Constructs a IpeRenderer for the given painting.
	IpeRenderer(const std::shared_ptr<GeometryPainting>& painting);

	/// Saves the painting to an Ipe file with the given name.
	void save(const std::filesystem::path& file);

	void draw(const Point<Inexact>& p) override;
	void draw(const Segment<Inexact>& s) override;
	void draw(const Polygon<Inexact>& p) override;
	void draw(const PolygonWithHoles<Inexact>& p) override;
	void draw(const Circle<Inexact>& c) override;
	void draw(const BezierSpline& s) override;
	void draw(const Line<Inexact>& l) override;
	void draw(const Ray<Inexact>& r) override;
	void draw(const Polyline<Inexact>& p) override;
	void drawText(const Point<Inexact>& p, const std::string& text) override;

	void pushStyle() override;
	void popStyle() override;
	void setMode(int mode) override;
	void setStroke(Color color, double width) override;
	void setFill(Color color) override;
	void setFillOpacity(int alpha) override;

	void addPainting(std::shared_ptr<GeometryPainting> painting);

  private:
	/// Converts a polygon to an Ipe curve.
	ipe::Curve* convertPolygonToCurve(const Polygon<Inexact>& p) const;
	/// Converts a polyline to an Ipe curve.
	ipe::Curve* convertPolylineToCurve(const Polyline<Inexact>& p) const;
	/// Returns Ipe attributes to style an Ipe path with the current style.
	ipe::AllAttributes getAttributesForStyle() const;
	/// Escapes LaTeX's [reserved characters](https://latexref.xyz/Reserved-characters.html)
	/// `# $ % & { } _ ~ ^ \` so that the resulting string can safely be used in
	/// an Ipe file.
	std::string escapeForLaTeX(const std::string& text) const;

	/// The paintings we're drawing.
	std::vector<std::shared_ptr<GeometryPainting>> m_paintings;
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
	/// The index of the Ipe layer we are currently drawing to.
	int m_layer;
};

} // namespace cartocrow::renderer

#endif //CARTOCROW_RENDERER_IPE_RENDERER_H
