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

#ifndef CARTOCROW_RENDERER_STORED_PAINTING_H
#define CARTOCROW_RENDERER_STORED_PAINTING_H

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
		/// The color of filled shapes.
		Color m_fillColor = Color{0, 102, 203};
		/// The opacity of filled shapes.
		double m_fillOpacity = 255;
	};
	using Label = std::pair<Point<Inexact>, std::string>;
	using DrawableObject = std::variant<Point<Inexact>, Segment<Inexact>, Polygon<Inexact>,
	                                    PolygonWithHoles<Inexact>, Circle<Inexact>, BezierSpline,
	                                    Line<Inexact>, Ray<Inexact>, Polyline<Inexact>, Label, Style>;
	std::vector<DrawableObject> m_objects;
	Style m_style;
};

} // namespace cartocrow::renderer

#endif //CARTOCROW_RENDERER_STORED_PAINTING_H
