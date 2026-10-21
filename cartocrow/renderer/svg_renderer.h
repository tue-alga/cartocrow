/*
Copyright (C) 2026  TU Eindhoven

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

#pragma once

#include <filesystem>
#include <fstream>
#include <stack>

#include "geometry_painting.h"
#include "geometry_renderer.h"

namespace cartocrow::renderer {

/// The style for a GeometryWidget.
struct SvgRendererStyle {
	/// The draw mode.
	int m_mode = GeometryRenderer::stroke;
	/// The diameter of points.
	double m_pointSize = 10;
	/// Whether the size is interpreted as absolute, that is, independent of
	/// the renderer's zoom factor.
	bool m_absoluteSize = false;
	/// The color of points and lines.
	std::string m_strokeColor = "#000000";
	/// The opacity of points and lines.
	double m_strokeOpacity = 1;
	/// The width of lines.
	double m_strokeWidth = 1;
	/// Whether the width is interpreted as absolute, that is, independent of
	/// the renderer's zoom factor.
	bool m_absoluteWidth = false;
	/// The color of filled shapes.
	std::string m_fillColor = "#0066cb";
	/// The opacity of filled shapes.
	double m_fillOpacity = 1;
    /// Whether shapes are clipped by the current clip path.
    bool m_clipping = false;
    /// The index of the current clip path, if any.
    std::optional<int> m_clipPath;
    /// Line cap.
    std::string m_lineCap = "round";
    /// Line join.
    std::string m_lineJoin = "round";
    /// Horizontal text alignment.
    std::string m_horizontalTextAlignment = "middle";
    /// Vertical text alignment.
    std::string m_verticalTextAlignment = "middle";
	/// Bold text?
	bool m_bold = false;
	/// Local (per text element) font size
	std::optional<double> m_fontSize = std::nullopt;
	/// Local (per text element) font family
	std::optional<std::string> m_fontFamily = std::nullopt;
};

/// SVG specialization of the GeometryRenderer.
class SvgRenderer : public GeometryRenderer {

  public:
	SvgRenderer() = default;

	/// Constructs a SvgRenderer for the given painting.
	SvgRenderer(const std::shared_ptr<GeometryPainting>& painting);
	SvgRenderer(const std::shared_ptr<GeometryPainting>& painting, const std::string& name);

	/// Saves the painting to an SVG file with the given name.
	void save(const std::filesystem::path& file);
	void save(const std::filesystem::path& file, Box viewBox);

	void draw(const Point<Inexact>& p) override;
	void draw(const Circle<Inexact>& c) override;
	void draw(const CubicBezierSpline& s) override;
	void draw(const Ellipse& e) override;
	void draw(const Line<Inexact>& l) override;
	void draw(const Ray<Inexact>& r) override;
	void draw(const Halfplane<Inexact>& h) override;
	void draw(const RenderPath& p) override;
	void drawText(const Point<Inexact>& p, const std::string& text, bool escape=true) override;

	void pushStyle() override;
	void popStyle() override;
	void setMode(int mode) override;
	void setStroke(Color color, double width, bool absoluteWidth = false) override;
	void setStrokeOpacity(int alpha) override;
	void setFill(Color color) override;
	void setFillOpacity(int alpha) override;
	void setPointSize(double size, bool absoluteSize = false) override;
    void setClipPath(const RenderPath& clipPath) override;
    void setClipping(bool enable) override;
	void setLineJoin(LineJoin lineJoin) override;
	void setLineCap(LineCap lineCap) override;
	void setHorizontalTextAlignment(HorizontalTextAlignment alignment) override;
	void setVerticalTextAlignment(VerticalTextAlignment alignment) override;
	void setFontFamily(std::string fontFamily) override;
	void setFontSize(double fontSize) override;
	void setFontWeight(bool bold) override;
	void useDefaultFont() override;

	void addPainting(const std::shared_ptr<GeometryPainting>& painting);
	void addPainting(const std::shared_ptr<GeometryPainting>& painting, const std::string& name);
	void addPainting(const std::function<void(renderer::GeometryRenderer&)>& draw_function);
	void addPainting(const std::function<void(renderer::GeometryRenderer&)>& draw_function, const std::string& name);

  private:
	/// The output file we're writing to.
	std::ofstream m_out;
	/// Converts a polygon to an SVG path specification.
	std::string convertPolygonToCurve(const Polygon<Inexact>& p) const;
	/// Returns the style as a string that can be added to an SVG path element.
	std::string getStyle() const;
	/// Returns the style as a string that can be added to an SVG use element.
	std::string getVertexStyle() const;
	/// Escapes SVG's reserved characters.
	std::string escapeForSvg(const std::string& text) const;

	struct DrawnPainting {
		/// The painting itself.
		std::shared_ptr<GeometryPainting> m_painting;
		/// The name of the painting displayed as a layer name in Inkscape.
		///
		/// The SVG standard does not have a concept of layers, however Inkscape
		/// does support them by means of groups with the custom inkscape:label
		/// attribute.
		std::optional<std::string> name;
	};

	/// The paintings we're drawing.
	std::vector<DrawnPainting> m_paintings;
	/// The current drawing style.
	SvgRendererStyle m_style;
	/// A stack of drawing styles, used by \ref pushStyle() and \ref popStyle()
	/// to store previously pushed styles.
	std::stack<SvgRendererStyle> m_styleStack;
    /// Clip path index; the number of clip paths already added to the SVG.
    int m_clipPathId = 0;

	/// Output paintings as svg to m_out.
	void savePaintings();
};

std::string renderPathToSVGCommands(const RenderPath& p, bool invertY = true);
} // namespace cartocrow::renderer
