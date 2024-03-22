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

#include <QListWidget>
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
	int m_mode = GeometryRenderer::stroke;
	/// The diameter of points.
	double m_pointSize = 10;
	/// The color of points and lines.
	QColor m_strokeColor = QColor(0, 0, 0);
	/// The width of lines.
	double m_strokeWidth = 1;
	/// The color of filled shapes.
	QColor m_fillColor = QColor(0, 102, 203);
};

/// \ref QWidget specialization of the GeometryRenderer.
///
/// A GeometryWidget renders the GeometryPainting using a \ref QPainter. It is
/// well-suited to display a painting for debugging purposes, and for this
/// purpose it supports a number of interactivity features. The user is able to
/// pan and zoom the canvas, and the coordinate of the mouse cursor is shown in
/// the bottom-right corner. Additionally, GeometryWidget automatically draws a
/// Cartesian coordinate system behind the painting (this can be turned off
/// using \ref setDrawAxes().
///
/// A GeometryWidget allows showing more than one painting at a time. These
/// layers can be named (see \ref addPainting()) and if there is more than one,
/// the user is able to toggle the visibility of each one individually.
///
/// It is very simple to create a GeometryWidget for a given painting and use it
/// for debugging, for example like this:
///
/// ```cpp
/// // std::shared_ptr<GeometryPainting> painting = ...;
/// QApplication app(argc, argv);
/// GeometryWidget widget = new GeometryWidget(painting);
/// widget->show();
/// app.exec();
/// ```
///
/// ## Editables
///
/// GeometryWidget allows the user to edit inputs for an algorithm. Editable
/// geometry objects are called *editables*. You can register an editable using
/// \ref registerEditable(). Connect to the \ref edited() signal to be notified
/// when the user edits something, so that you can rerun the algorithm.
class GeometryWidget : public QWidget, GeometryRenderer {
	Q_OBJECT;

  public:
	/// Types of grid.
	enum class GridMode {
		/// A cartesian \f$(x, y)\f$ grid.
		CARTESIAN,
		/// A polar \f$(r, \theta)\f$ grid.
		POLAR
	};

	/// A geometry object that can be edited by the user.
	class Editable {
	  public:
		Editable(GeometryWidget* widget);
		/// Draws a hint that this editable will be active when the user starts
		/// dragging from this location. Returns `false` if the editable
		/// wouldn't become active from this location.
		virtual bool drawHoverHint(Point<Inexact> location, Number<Inexact> radius) const = 0;
		/// Starts a drag operation. Returns `false` if the editable doesn't
		/// want to become active from this location.
		virtual bool startDrag(Point<Inexact> location, Number<Inexact> radius) = 0;
		/// Handles a drag operation.
		virtual void handleDrag(Point<Inexact> to) const = 0;
		/// Ends a running drag operation.
		virtual void endDrag() = 0;

	  protected:
		/// The GeometryWidget we belong to.
		GeometryWidget* m_widget;
	};

	/// Editable for \ref Point<Inexact>.
	class PointEditable : public Editable {
	  public:
		/// Constructs a PointEditable for the given point.
		PointEditable(GeometryWidget* widget, std::shared_ptr<Point<Inexact>> point);
		bool drawHoverHint(Point<Inexact> location, Number<Inexact> radius) const override;
		bool startDrag(Point<Inexact> location, Number<Inexact> radius) override;
		void handleDrag(Point<Inexact> to) const override;
		void endDrag() override;

	  private:
		/// Checks if the location is within a circle with the given radius
		/// around the point.
		bool isClose(Point<Inexact> location, Number<Inexact> radius) const;
		/// The point we're editing.
		std::shared_ptr<Point<Inexact>> m_point;
	};

	class PolygonEditable : public Editable {
	  public:
		/// Constructs a PolygonEditable for the given polygon.
		PolygonEditable(GeometryWidget* widget, std::shared_ptr<Polygon<Inexact>> polygon);
		bool drawHoverHint(Point<Inexact> location, Number<Inexact> radius) const override;
		bool startDrag(Point<Inexact> location, Number<Inexact> radius) override;
		void handleDrag(Point<Inexact> to) const override;
		void endDrag() override;

	  private:
		/// Finds a polygon vertex that is within a circle with the given radius
		/// around the point. If such a vertex doesn't exist, this returns `-1`.
		int findVertex(Point<Inexact> location, Number<Inexact> radius) const;
		/// The polygon we're editing.
		std::shared_ptr<Polygon<Inexact>> m_polygon;
		int m_draggedVertex = -1;
	};

	/// Constructs a GeometryWidget without a painting.
	GeometryWidget();
	/// Constructs a GeometryWidget for the given painting.
	GeometryWidget(std::shared_ptr<GeometryPainting> painting);

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

	/// Adds a new painting to this widget.
	void addPainting(std::shared_ptr<GeometryPainting> painting, const std::string& name);
	/// Removes all paintings from this widget.
	void clear();

	/// Returns the current zoom factor, in pixels per unit.
	Number<Inexact> zoomFactor() const;

	/// Adds an editable point.
	void registerEditable(std::shared_ptr<Point<Inexact>> point);
	/// Adds an editable polygon.
	void registerEditable(std::shared_ptr<Polygon<Inexact>> polygon);

  public slots:
	/// Determines whether to draw the axes and gridlines in the background.
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
	/// Sets the type of grid.
	void setGridMode(GridMode mode);

  signals:
	/// Emitted when the user clicks on the widget.
	void clicked(Point<Inexact> location);
	/// Emitted when the user started dragging with the mouse.
	void dragStarted(Point<Inexact> location);
	/// Emitted when the user moved the mouse over the widget while holding the
	/// mouse button.
	void dragMoved(Point<Inexact> location);
	/// Emitted when the user stopped dragging with the mouse.
	void dragEnded(Point<Inexact> location);
	/// Emitted when the user edited an editable.
	void edited();

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

	/// Sets the pen and brush on \ref m_painter corresponding to \link m_style.
	void setupPainter();

	/// Draws the axes, grid, and axis labels in the background, taking the
	/// current zoom level into account.
	void drawAxes();
	/// Draws the coordinate hovered by the mouse.
	void drawCoordinates();
	/// Moves the zoom slider knob to the currently set zoom level.
	void updateZoomSlider();
	/// Puts the layers currently in this GeometryWidget into the layer list.
	void updateLayerList();
	/// Data about a painting we're drawing.
	struct DrawnPainting {
		/// The painting itself.
		std::shared_ptr<GeometryPainting> m_painting;
		/// The name of the painting, to display in the layers pane.
		std::string name;
		/// Whether the painting is currently visible.
		bool visible;
	};
	/// The set of layer names that were invisible. This set doesn't get cleared
	/// when paintings are removed; when a new painting is added its name is
	/// looked up in the set to find out if it should be made visible initially.
	std::set<std::string> m_invisibleLayerNames;
	/// List of the paintings we're drawing.
	std::vector<DrawnPainting> m_paintings;
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
	/// Whether a panning operation is in progress.
	bool m_panning = false;
	/// Whether a dragging operation is in progress.
	bool m_dragging = false;
	/// Whether the mouse button is currently held.
	bool m_mouseButtonDown = false;
	/// Whether to draw the background axes.
	bool m_drawAxes = true;
	/// The grid mode.
	GridMode m_gridMode = GridMode::CARTESIAN;
	/// The registered editables.
	std::vector<std::unique_ptr<Editable>> m_editables;
	/// The editable in \ref m_editables that the user is currently interacting
	/// with, or `nullptr` if no such interaction is going on.
	Editable* m_activeEditable = nullptr;
	
	/// The current drawing style.
	GeometryWidgetStyle m_style;
	/// A stack of drawing styles, used by \link pushStyle() and \link
	/// popStyle() to store previously pushed styles.
	std::stack<GeometryWidgetStyle> m_styleStack;
	/// The list of layers, allowing the user to toggle their visibility.
	QListWidget* m_layerList;
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
