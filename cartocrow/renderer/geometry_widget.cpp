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

#include "geometry_widget.h"

namespace cartocrow {
namespace renderer {

GeometryWidget::GeometryWidget(GeometryPainting& painting) :
		m_painting(painting) {
}

void GeometryWidget::paintEvent(QPaintEvent* event) {
	m_painter = std::make_unique<QPainter>(this);
	m_painter->setRenderHint(QPainter::Antialiasing);
	m_painting.paint(*this);
	m_painter->end();
}

void GeometryWidget::draw(cartocrow::Point p) {
	m_painter->setPen(Qt::NoPen);
	m_painter->setBrush(m_strokeColor);
	m_painter->drawEllipse(QRect(p.x() - 0.5 * m_pointSize, p.y() - 0.5 * m_pointSize,
			m_pointSize, m_pointSize));
}

void GeometryWidget::pushStyle() {
	// TODO
}

void GeometryWidget::popStyle() {
	// TODO
}

void GeometryWidget::setStroke(Color color, double width) {
	m_strokeColor = QColor(color.r, color.g, color.b);
	m_strokeWidth = width;
}

std::unique_ptr<QPainter> GeometryWidget::getQPainter() {
	return std::make_unique<QPainter>(this);
}

} // namespace renderer
} // namespace cartocrow
