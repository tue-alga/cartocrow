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

#include "painting_renderer.h"

#include <variant>

namespace cartocrow::renderer {

PaintingRenderer::PaintingRenderer() {}

void PaintingRenderer::paint(GeometryRenderer& renderer) const {
    Style lastStyle;

	for (auto& object : m_objects) {
		if (std::holds_alternative<Point<Inexact>>(object)) {
			renderer.draw(std::get<Point<Inexact>>(object));
		} else if (std::holds_alternative<Circle<Inexact>>(object)) {
			renderer.draw(std::get<Circle<Inexact>>(object));
		} else if (std::holds_alternative<BezierSpline>(object)) {
			renderer.draw(std::get<BezierSpline>(object));
		} else if (std::holds_alternative<Line<Inexact>>(object)) {
			renderer.draw(std::get<Line<Inexact>>(object));
		} else if (std::holds_alternative<Ray<Inexact>>(object)) {
			renderer.draw(std::get<Ray<Inexact>>(object));
		} else if (std::holds_alternative<Halfplane<Inexact>>(object)) {
			renderer.draw(std::get<Halfplane<Inexact>>(object));
		} else if (std::holds_alternative<RenderPath>(object)) {
			renderer.draw(std::get<RenderPath>(object));
		} else if (std::holds_alternative<Label>(object)) {
			auto& [text, position, escape] = std::get<Label>(object);
			renderer.drawText(text, position, escape);
		} else if (std::holds_alternative<Style>(object)) {
			Style style = std::get<Style>(object);
			renderer.setFill(style.m_fillColor);
			renderer.setFillOpacity(style.m_fillOpacity);
			renderer.setStroke(style.m_strokeColor, style.m_strokeWidth, style.m_absoluteWidth);
			renderer.setStrokeOpacity(style.m_strokeOpacity);
			renderer.setClipping(style.m_clip);
            if (style.m_clipPath != lastStyle.m_clipPath) {
			    renderer.setClipPath(style.m_clipPath);
            }
			renderer.setHorizontalTextAlignment(style.m_horizontalTextAlignment);
			renderer.setVerticalTextAlignment(style.m_verticalTextAlignment);
			renderer.setLineCap(style.m_lineCap);
			renderer.setLineJoin(style.m_lineJoin);
			renderer.setMode(style.m_mode);

            lastStyle = style;
		}
	}
}

void PaintingRenderer::draw(const Point<Inexact>& p) {
	m_objects.push_back(p);
}

void PaintingRenderer::draw(const Circle<Inexact>& c) {
	m_objects.push_back(c);
}

void PaintingRenderer::draw(const BezierSpline& s) {
	m_objects.push_back(s);
}

void PaintingRenderer::draw(const Line<Inexact>& l) {
	m_objects.push_back(l);
}

void PaintingRenderer::draw(const Ray<Inexact>& r) {
	m_objects.push_back(r);
}

void PaintingRenderer::draw(const Halfplane<Inexact>& h) {
	m_objects.push_back(h);
}

void PaintingRenderer::draw(const RenderPath& p) {
	m_objects.push_back(p);
}

void PaintingRenderer::drawText(const Point<Inexact>& p, const std::string& text, bool escape) {
	m_objects.push_back(Label(p, text, escape));
}

void PaintingRenderer::pushStyle() {
	m_styleStack.push(m_style);
}

void PaintingRenderer::popStyle() {
	m_style = m_styleStack.top();
	m_styleStack.pop();
}

void PaintingRenderer::setMode(int mode) {
	m_style.m_mode = mode;
	m_objects.push_back(m_style);
}

void PaintingRenderer::setStroke(Color color, double width, bool absoluteWidth) {
	m_style.m_strokeColor = color;
	m_style.m_strokeWidth = width;
	m_style.m_absoluteWidth = absoluteWidth;
	m_objects.push_back(m_style);
}

void PaintingRenderer::setStrokeOpacity(int alpha) {
	m_style.m_strokeOpacity = alpha;
	m_objects.push_back(m_style);
}

void PaintingRenderer::setFill(Color color) {
	m_style.m_fillColor = color;
	m_objects.push_back(m_style);
}

void PaintingRenderer::setFillOpacity(int alpha) {
	m_style.m_fillOpacity = alpha;
	m_objects.push_back(m_style);
}

void PaintingRenderer::setClipPath(const RenderPath& clipPath) {
	m_style.m_clipPath = clipPath;
    m_objects.push_back(m_style);
}

void PaintingRenderer::setClipping(bool enable) {
	m_style.m_clip = enable;
    m_objects.push_back(m_style);
}

void PaintingRenderer::setLineJoin(LineJoin lineJoin) {
	m_style.m_lineJoin = lineJoin;
    m_objects.push_back(m_style);
}

void PaintingRenderer::setLineCap(LineCap lineCap) {
	m_style.m_lineCap = lineCap;
    m_objects.push_back(m_style);
}

void PaintingRenderer::setHorizontalTextAlignment(HorizontalTextAlignment alignment) {
	m_style.m_horizontalTextAlignment = alignment;
    m_objects.push_back(m_style);
}

void PaintingRenderer::setVerticalTextAlignment(VerticalTextAlignment alignment) {
	m_style.m_verticalTextAlignment = alignment;
    m_objects.push_back(m_style);
}
} // namespace cartocrow::renderer
