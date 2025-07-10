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

#ifndef CARTOCROW_RENDERER_PAINTING_RENDERER_H
#define CARTOCROW_RENDERER_PAINTING_RENDERER_H

#include "geometry_painting.h"
#include "geometry_renderer.h"

namespace cartocrow::renderer {

/// Renderer that does not actually render, but instead serves as a painting
/// that stores the render commands executed, so that they can later be rendered
/// by another renderer. This is meant to be used for debug drawing and so on.
class PaintingRenderer : public GeometryPainting, public GeometryRenderer {

  public:
	/// Creates a new painting renderer.
	PaintingRenderer();

	void paint(GeometryRenderer& renderer) const override;

	void draw(const Point<Inexact>& p) override;
	void draw(const Circle<Inexact>& c) override;
	void draw(const BezierSpline& s) override;
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
	void setClipPath(const RenderPath& clipPath) override;
	void setClipping(bool enable) override;
	void setLineJoin(LineJoin lineJoin) override;
	void setLineCap(LineCap lineCap) override;
	void setHorizontalTextAlignment(HorizontalTextAlignment alignment) override;
	void setVerticalTextAlignment(VerticalTextAlignment alignment) override;

  private:
	struct Style {
		/// The draw mode.
		int m_mode = GeometryRenderer::stroke;
		/// The diameter of points.
		double m_pointSize = 10;
		/// The color of points and lines.
		Color m_strokeColor = Color{0, 0, 0};
		/// The width of lines.
		double m_strokeWidth = 1;
		/// Whether the width is interpreted as absolute, that is, independent of
		/// the renderer's zoom factor.
		bool m_absoluteWidth = false;
		/// The opacity of lines.
		double m_strokeOpacity = 255;
		/// The color of filled shapes.
		Color m_fillColor = Color{0, 102, 203};
		/// The opacity of filled shapes.
		double m_fillOpacity = 255;
		/// The current clip path
		RenderPath m_clipPath;
		/// Clipping enabled?
		bool m_clip = false;
		/// Current line join.
		LineJoin m_lineJoin = RoundJoin;
		/// Current line cap.
		LineCap m_lineCap = RoundCap;
		/// Horizontal text alignment
		HorizontalTextAlignment m_horizontalTextAlignment = AlignHCenter;
		/// Vertical text alignment
		VerticalTextAlignment m_verticalTextAlignment = AlignVCenter;
	};
	using Label = std::tuple<Point<Inexact>, std::string, bool>;
	using DrawableObject =
	    std::variant<Point<Inexact>, Circle<Inexact>, BezierSpline,
	                 Line<Inexact>, Ray<Inexact>, Halfplane<Inexact>, RenderPath, Label, Style>;
	std::vector<DrawableObject> m_objects;
	Style m_style;
	std::stack<Style> m_styleStack;
};

} // namespace cartocrow::renderer

#endif //CARTOCROW_RENDERER_PAINTING_RENDERER_H
