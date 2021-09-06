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

#include <stack>

#include "geometry_painting.h"
#include "geometry_renderer.h"

namespace cartocrow {
namespace renderer {

/**
 * The style for a GeometryWidget.
 */
struct GeometryWidgetStyle {

	/**
	 * The diameter of points.
	 */
	double m_pointSize = 10;

	/**
	 * The color of points and lines.
	 */
	QColor m_strokeColor = QColor(0, 0, 0);

	/**
	 * The width of lines.
	 */
	double m_strokeWidth = 1;

	/**
	 * The color of filled shapes.
	 */
	QColor m_fillColor = QColor(0, 0, 0);
};

/**
 * QWidget specialization of the GeometryRenderer.
 */
class GeometryWidget : public QWidget, GeometryRenderer {

  public:
	/**
	 * Constructs a GeometryWidget for the given painting.
	 */
	GeometryWidget(GeometryPainting& painting);

	void draw(Point p) override;
	void draw(Segment s) override;
	void pushStyle() override;
	void popStyle() override;
	void setStroke(Color color, double width) override;
	std::unique_ptr<QPainter> getQPainter() override;

  public slots:
	/**
	 * Determines whether to draw the axes in the background.
	 */
	void setDrawAxes(bool drawAxes);

	/**
	 * Sets the minimum zoom level, in pixels per unit. If the current zoom
	 * level violates the minimum, it is not automatically adjusted.
	 */
	void setMinZoom(double minZoom);

	/**
	 * Sets the maximum zoom level, in pixels per unit. If the current zoom
	 * level violates the maximum, it is not automatically adjusted.
	 */
	void setMaxZoom(double maxZoom);

  protected:
	void paintEvent(QPaintEvent* event) override final;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;
	void leaveEvent(QEvent* event) override;

  private:
    /**
	 * Converts a point in drawing coordinates to Qt coordinates.
	 */
	QPointF convertPoint(Point p) const;

    /**
	 * Converts a point in drawing coordinates to Qt coordinates.
	 */
	QPointF convertPoint(double x, double y) const;

    /**
	 * Converts a point in Qt coordinates back to drawing coordinates.
	 */
	Point inverseConvertPoint(QPointF p) const;

    /**
	 * Converts a rectangle in Qt coordinates back to drawing coordinates.
	 */
	Box inverseConvertBox(QRectF r) const;

	/**
	 * Draws the axes, grid, and axis labels in the background, taking the
	 * current zoom level into account.
	 */
	void drawAxes();

	/**
	 * The painting we're drawing.
	 */
	GeometryPainting& m_painting;

	/**
	 * The QPainter we are drawing with. Only valid while painting.
	 */
	std::unique_ptr<QPainter> m_painter;

	/**
	 * The transform from drawing coordinates to Qt coordinates.
	 */
	QTransform m_transform;

	/**
	 * The zoom lower bound, in pixels per unit.
	 */
	double m_minZoom = 0.1;

	/**
	 * The zoom upper bound, in pixels per unit.
	 */
	double m_maxZoom = 300.0;

	/**
	 * The current mouse position, in Qt coordinates.
	 */
	QPointF m_mousePos;

	/**
	 * The previous mouse position, in Qt coordinates. Used together with
	 * \link m_mousePos to figure out how far the mouse was moved during a drag.
	 */
	QPointF m_previousMousePos;

	/**
	 * Whether a dragging operation is in progress.
	 */
	bool m_dragging = false;

	/**
	 * Whether to draw the background axes.
	 */
	bool m_drawAxes = true;

	/**
	 * The current drawing style.
	 */
	GeometryWidgetStyle m_style;
	
	/**
	 * A stack of drawing styles, used by \link pushStyle() and \link popStyle()
	 * to store previously pushed styles.
	 */
	std::stack<GeometryWidgetStyle> m_styleStack;
};

} // namespace renderer
} // namespace cartocrow

#endif //CARTOCROW_GEOMETRY_WIDGET
