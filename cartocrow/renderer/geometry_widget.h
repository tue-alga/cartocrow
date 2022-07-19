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

#ifndef CARTOCROW_RENDERER_GEOMETRY_WIDGET_H
#define CARTOCROW_RENDERER_GEOMETRY_WIDGET_H

#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QSlider>
#include <QToolBar>
#include <QToolButton>
#include <QTransform>
#include <QWheelEvent>
#include <QWidget>

#include <stack>

#include "geometry_painting.h"
#include "geometry_renderer.h"

namespace cartocrow::renderer {

/// The style for a GeometryWidget.
struct GeometryWidgetStyle {
	/// The draw mode.
	int m_mode = GeometryRenderer::stroke | GeometryRenderer::fill;
	/// The diameter of points.
	double m_pointSize = 10;
	/// The color of points and lines.
	QColor m_strokeColor = QColor(0, 0, 0);
	/// The width of lines.
	double m_strokeWidth = 1;
	/// The color of filled shapes.
	QColor m_fillColor = QColor(0, 102, 203);
};

/// QWidget specialization of the GeometryRenderer.
class GeometryWidget : public QWidget, GeometryRenderer {

  public:
	/// Constructs a GeometryWidget for the given painting.
	GeometryWidget(const GeometryPainting& painting);

	void draw(const Point<Inexact>& p) override;
	void draw(const Segment<Inexact>& s) override;
	void draw(const Polygon<Inexact>& p) override;
	void draw(const PolygonWithHoles<Inexact>& p) override;
	void draw(const Circle<Inexact>& c) override;
	//void draw(const BezierSpline& s) override;
	void drawText(const Point<Inexact>& p, const std::string& text) override;

	void pushStyle() override;
	void popStyle() override;
	void setMode(int mode) override;
	void setStroke(Color color, double width) override;
	void setFill(Color color) override;
	void setFillOpacity(int alpha) override;

  public slots:
	/// Determines whether to draw the axes in the background.
	void setDrawAxes(bool drawAxes);
	/// Sets the minimum zoom level, in pixels per unit. If the current zoom
	/// level violates the minimum, it is not automatically adjusted.
	void setMinZoom(double minZoom);
	/// Sets the maximum zoom level, in pixels per unit. If the current zoom
	/// level violates the maximum, it is not automatically adjusted.
	void setMaxZoom(double maxZoom);
	/// Increases the zoom level, taking the maximum zoom into account.
	void zoomIn();
	/// Decreases the zoom level, taking the minimum zoom into account.
	void zoomOut();

  protected:
	void resizeEvent(QResizeEvent* e) override;
	void paintEvent(QPaintEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;
	void leaveEvent(QEvent* event) override;
	QSize sizeHint() const override;

	/// Converts a point in drawing coordinates to Qt coordinates.
	QPointF convertPoint(Point<Inexact> p) const;
	/// Converts a rectangle in drawing coordinates to Qt coordinates.
	QRectF convertBox(Box b) const;
	/// Converts a point in Qt coordinates back to drawing coordinates.
	Point<Inexact> inverseConvertPoint(QPointF p) const;
	/// Converts a rectangle in Qt coordinates back to drawing coordinates.
	Box inverseConvertBox(QRectF r) const;

  private:
	/// Converts the polygon to Qt coordinates and adds it to the QPainterPath.
	void addPolygonToPath(QPainterPath& path, const Polygon<Inexact>& p);

	/// Sets the pen and brush on \link m_painter corresponding to \link
	/// m_style.
	void setupPainter();

	/// Draws the axes, grid, and axis labels in the background, taking the
	/// current zoom level into account.
	void drawAxes();
	/// Moves the zoom slider knob to the currently set zoom level.
	void updateZoomSlider();
	/// The painting we're drawing.
	const GeometryPainting& m_painting;
	/// The QPainter we are drawing with. Only valid while painting.
	std::unique_ptr<QPainter> m_painter;
	/// The transform from drawing coordinates to Qt coordinates.
	QTransform m_transform;
	/// The zoom lower bound, in pixels per unit.
	double m_minZoom = 0.1;
	/// The zoom upper bound, in pixels per unit.
	double m_maxZoom = 300.0;
	/// The current mouse position, in Qt coordinates.
	QPointF m_mousePos;
	/// The previous mouse position, in Qt coordinates. Used together with
	/// \link m_mousePos to figure out how far the mouse was moved during a drag.
	QPointF m_previousMousePos;
	/// Whether a dragging operation is in progress.
	bool m_dragging = false;
	/// Whether to draw the background axes.
	bool m_drawAxes = true;
	/// The current drawing style.
	GeometryWidgetStyle m_style;
	/// A stack of drawing styles, used by \link pushStyle() and \link popStyle()
	/// to store previously pushed styles.
	std::stack<GeometryWidgetStyle> m_styleStack;
	/// The toolbar containing the zoom buttons.
	QToolBar* m_zoomBar;
	/// The zoom out button in the toolbar.
	QToolButton* m_zoomOutButton;
	/// The zoom slider.
	QSlider* m_zoomSlider;
	/// The zoom in button in the toolbar.
	QToolButton* m_zoomInButton;
};

} // namespace cartocrow::renderer

#endif //CARTOCROW_RENDERER_GEOMETRY_WIDGET_H
