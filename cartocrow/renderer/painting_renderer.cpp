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
	for (auto& object : m_objects) {
		if (std::holds_alternative<Point<Inexact>>(object)) {
			renderer.draw(std::get<Point<Inexact>>(object));
		} else if (std::holds_alternative<Segment<Inexact>>(object)) {
			renderer.draw(std::get<Segment<Inexact>>(object));
		} else if (std::holds_alternative<Polygon<Inexact>>(object)) {
			renderer.draw(std::get<Polygon<Inexact>>(object));
		} else if (std::holds_alternative<PolygonWithHoles<Inexact>>(object)) {
			renderer.draw(std::get<PolygonWithHoles<Inexact>>(object));
		} else if (std::holds_alternative<Circle<Inexact>>(object)) {
			renderer.draw(std::get<Circle<Inexact>>(object));
		} else if (std::holds_alternative<Label>(object)) {
			renderer.drawText(std::get<Label>(object).first, std::get<Label>(object).second);
		} else if (std::holds_alternative<Style>(object)) {
			Style style = std::get<Style>(object);
			renderer.setFill(style.m_fillColor);
			renderer.setFillOpacity(style.m_fillOpacity);
			renderer.setStroke(style.m_strokeColor, style.m_strokeWidth);
			renderer.setMode(style.m_mode);
		}
	}
}

void PaintingRenderer::draw(const Point<Inexact>& p) {
	m_objects.push_back(p);
}

void PaintingRenderer::draw(const Segment<Inexact>& s) {
	m_objects.push_back(s);
}

void PaintingRenderer::draw(const Polygon<Inexact>& p) {
	m_objects.push_back(p);
}

void PaintingRenderer::draw(const PolygonWithHoles<Inexact>& p) {
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

void PaintingRenderer::draw(const Polyline<Inexact>& p) {
	m_objects.push_back(p);
}

void PaintingRenderer::drawText(const Point<Inexact>& p, const std::string& text) {
	m_objects.push_back(Label(p, text));
}

void PaintingRenderer::pushStyle() {
	//m_styleStack.push(m_style);
}

void PaintingRenderer::popStyle() {
	//m_style = m_styleStack.top();
	//m_styleStack.pop();
}

void PaintingRenderer::setMode(int mode) {
	m_style.m_mode = mode;
	m_objects.push_back(m_style);
}

void PaintingRenderer::setStroke(Color color, double width) {
	m_style.m_strokeColor = color;
	m_style.m_strokeWidth = width;
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

} // namespace cartocrow::renderer
