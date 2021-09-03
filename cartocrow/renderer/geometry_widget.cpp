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

#include <QtGlobal>
#include <QPoint>

namespace cartocrow {
namespace renderer {

GeometryWidget::GeometryWidget(GeometryPainting& painting) : m_painting(painting) {
	setMouseTracking(true);
}

void GeometryWidget::paintEvent(QPaintEvent* event) {
	m_painter = std::make_unique<QPainter>(this);
	m_painter->setRenderHint(QPainter::Antialiasing);
	if (m_drawBackground) {
		m_painter->fillRect(rect(), Qt::white);
	}
	m_painting.paint(*this);
	m_painter->end();
}

void GeometryWidget::mouseMoveEvent(QMouseEvent* event) {
	QPointF converted = inverseConvertPoint(event->pos());
	auto x = static_cast<int>(converted.x() + 0.5);
	auto y = static_cast<int>(converted.y() + 0.5);
	m_mousePos = QPointF(x, y);

	if (m_dragging) {
		QPointF delta = event->pos() - m_previousMousePos;
		QTransform translation;
		translation.translate(delta.x(), delta.y());
		m_transform = m_transform * translation;
	}

	m_previousMousePos = event->pos();
	update();
}

void GeometryWidget::mousePressEvent(QMouseEvent* event) {
	m_dragging = true;
	setCursor(Qt::ClosedHandCursor);
	m_previousMousePos = event->pos();
	update();
}

void GeometryWidget::mouseReleaseEvent(QMouseEvent* event) {
	m_dragging = false;
	setCursor(Qt::ArrowCursor);
}

void GeometryWidget::leaveEvent(QEvent* event) {
	// TODO?
	update();
}

void GeometryWidget::wheelEvent(QWheelEvent* event) {
	if (event->angleDelta().isNull()) {
		return;
	}

	double delta = event->angleDelta().y();
	double factor = pow(2, delta / 240);

	// limit the zoom factor
	// note: we cannot use limitZoomFactor() here, because then while it would
	// indeed be impossible to zoom in further than the MAX_ZOOM_FACTOR, the
	// translation would still be carried out
	if (factor * m_transform.m11() > m_maxZoom) {
		factor = m_maxZoom / m_transform.m11();
	}

	QPointF mousePos = event->position();
	mousePos -= QPointF(width() / 2.0, height() / 2.0);
	QTransform transform;
	transform.translate(mousePos.x(), mousePos.y());
	transform.scale(factor, factor);
	transform.translate(-mousePos.x(), -mousePos.y());
	m_transform *= transform;
	update();
}

QPointF GeometryWidget::convertPoint(Point p) const {
	return convertPoint(p.x(), p.y());
}

QPointF GeometryWidget::convertPoint(double x, double y) const {
	QPointF mapped = m_transform.map(QPointF(x + 0.5, y + 0.5));
	return mapped + QPointF(width(), height()) / 2.0;
}

QPointF GeometryWidget::inverseConvertPoint(QPointF p) const {
	QPointF toMap = p - QPointF(width(), height()) / 2.0;
	return m_transform.inverted().map(toMap) - QPointF(0.5, 0.5);
}

void GeometryWidget::draw(cartocrow::Point p) {
	m_painter->setPen(Qt::NoPen);
	m_painter->setBrush(m_strokeColor);
	QPointF p2 = convertPoint(p);
	m_painter->drawEllipse(
	    QRectF(p2.x() - 0.5 * m_pointSize, p2.y() - 0.5 * m_pointSize,
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

void GeometryWidget::setDrawBackground(bool drawBackground) {
	m_drawBackground = drawBackground;
	update();
}

} // namespace renderer
} // namespace cartocrow
