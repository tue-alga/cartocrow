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

#ifndef CARTOCROW_GEOMETRY_WIDGET
#define CARTOCROW_GEOMETRY_WIDGET

#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QTransform>
#include <QWheelEvent>
#include <QWidget>

#include "geometry_painting.h"
#include "geometry_renderer.h"

namespace cartocrow {
namespace renderer {

/**
 * QWidget specialization of the GeometryRenderer.
 */
class GeometryWidget : public QWidget, GeometryRenderer {

  public:
	GeometryWidget(GeometryPainting& painting);

	void draw(Point p) override;
	void pushStyle() override;
	void popStyle() override;
	void setStroke(Color color, double width) override;
	std::unique_ptr<QPainter> getQPainter() override;

  public slots:
	void setDrawBackground(bool drawBackground);
	void zoomIn();
	void zoomOut();
	void limitMaxZoom();

  protected:
	void paintEvent(QPaintEvent* event) override final;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;
	void leaveEvent(QEvent* event) override;

  private:
	QPointF convertPoint(Point p) const;
	QPointF convertPoint(double x, double y) const;
	QPointF inverseConvertPoint(QPointF p) const;
	GeometryPainting& m_painting;
	std::unique_ptr<QPainter> m_painter; // only valid while painting

	// transform
	QTransform m_transform;
	double m_minZoom = 0.1;
	double m_maxZoom = 30.0;
	QPointF m_mousePos;
	QPointF m_previousMousePos;
	bool m_dragging = false;

	// style
	double m_pointSize = 10;

	QColor m_strokeColor = QColor(0, 0, 0);
	double m_strokeWidth = 1;

	QColor m_fillColor = QColor(0, 0, 0);

	// other properties
	bool m_drawBackground = true;
};

} // namespace renderer
} // namespace cartocrow

#endif //CARTOCROW_GEOMETRY_WIDGET
