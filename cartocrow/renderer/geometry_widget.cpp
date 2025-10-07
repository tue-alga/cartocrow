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

#include "geometry_renderer.h"
#include "ipe_renderer.h"
#include "svg_renderer.h"

#include "function_painting.h"

#include <QFileDialog>
#include <QGuiApplication>
#include <QPainterPath>
#include <QPen>
#include <QPoint>
#include <QPolygon>
#include <QSlider>
#include <QToolButton>

#include <cmath>
#include <limits>

namespace cartocrow::renderer {

GeometryWidget::Editable::Editable(GeometryWidget* widget) : m_widget(widget) {}

GeometryWidget::PolygonEditable::PolygonEditable(GeometryWidget* widget, std::shared_ptr<Polygon<Inexact>> polygon)
    : Editable(widget), m_polygon(polygon) {}

bool GeometryWidget::PolygonEditable::drawHoverHint(Point<Inexact> location,
                                                    Number<Inexact> radius) const {
	int vertexId = findVertex(location, radius);
	if (vertexId == -1) {
		return false;
	}
	QPointF position = m_widget->convertPoint(*(m_polygon->vertices_begin() + vertexId));
	m_widget->m_painter->setPen(QPen(QBrush(QColor{240, 40, 20}), 1.5f));
	m_widget->m_painter->setBrush(Qt::NoBrush);
	m_widget->m_painter->drawEllipse(position, 5, 5);
	return true;
}

bool GeometryWidget::PolygonEditable::startDrag(Point<Inexact> location, Number<Inexact> radius) {
	int vertexId = findVertex(location, radius);
	if (vertexId == -1) {
		return false;
	}
	m_draggedVertex = vertexId;
	return true;
}

void GeometryWidget::PolygonEditable::handleDrag(Point<Inexact> to) const {
	assert(m_draggedVertex != -1);
	m_polygon->set(m_polygon->vertices_begin() + m_draggedVertex, to);
}

void GeometryWidget::PolygonEditable::endDrag() {
	m_draggedVertex = -1;
}

int GeometryWidget::PolygonEditable::findVertex(Point<Inexact> location,
                                                Number<Inexact> radius) const {
	for (int i = 0; i < m_polygon->size(); i++) {
		if ((*(m_polygon->vertices_begin() + i) - location).squared_length() < radius * radius) {
			return i;
		}
	}
	return -1;
}

GeometryWidget::PointEditable::PointEditable(GeometryWidget* widget, std::shared_ptr<Point<Inexact>> point)
    : Editable(widget), m_point(point) {}

bool GeometryWidget::PointEditable::drawHoverHint(Point<Inexact> location,
                                                    Number<Inexact> radius) const {
	if (!isClose(location, radius)) {
		return false;
	}
	QPointF position = m_widget->convertPoint(*m_point);
	m_widget->m_painter->setPen(QPen(QBrush(QColor{240, 40, 20}), 1.5f));
	m_widget->m_painter->setBrush(Qt::NoBrush);
	m_widget->m_painter->drawEllipse(position, 5, 5);
	return true;
}

bool GeometryWidget::PointEditable::startDrag(Point<Inexact> location, Number<Inexact> radius) {
	return isClose(location, radius);
}

void GeometryWidget::PointEditable::handleDrag(Point<Inexact> to) const {
	*m_point = to;
}

void GeometryWidget::PointEditable::endDrag() {}

bool GeometryWidget::PointEditable::isClose(Point<Inexact> location, Number<Inexact> radius) const {
	return (*m_point - location).squared_length() < radius * radius;
}

GeometryWidget::CircleEditable::CircleEditable(GeometryWidget* widget, std::shared_ptr<Circle<Inexact>> circle)
        : Editable(widget), m_circle(circle) {}

bool GeometryWidget::CircleEditable::drawHoverHint(Point<Inexact> location,
                                                  Number<Inexact> radius) const {
    if (isCloseToCenter(location, radius)) {
        QPointF position = m_widget->convertPoint(m_circle->center());
        m_widget->m_painter->setPen(QPen(QBrush(QColor{240, 40, 20}), 1.5f));
        m_widget->m_painter->setBrush(Qt::NoBrush);
        m_widget->m_painter->drawEllipse(position, 5, 5);
        return true;
    } else if (isCloseToBoundary(location, radius)) {
        m_widget->m_painter->setPen(QPen(QBrush(QColor{240, 40, 20}), 1.5f));
        m_widget->m_painter->setBrush(Qt::NoBrush);
        auto r = sqrt(m_circle->squared_radius());
        auto sx = m_widget->m_transform.m11();
        m_widget->m_painter->drawEllipse(m_widget->convertPoint(m_circle->center()), sx * r, sx * r);
        return true;
    } else {
        return false;
    }
}

bool GeometryWidget::CircleEditable::startDrag(Point<Inexact> location, Number<Inexact> radius) {
    if (isCloseToCenter(location, radius)) {
        m_dragging = Dragging::Center;
        return true;
    } else if (isCloseToBoundary(location, radius)) {
        m_dragging = Dragging::Radius;
        return true;
    }
    return false;
}

void GeometryWidget::CircleEditable::handleDrag(Point<Inexact> to) const {
    if (m_dragging == Dragging::Center) {
        *m_circle = Circle<Inexact>(to, m_circle->squared_radius());
    } else if (m_dragging == Dragging::Radius) {
        auto c = m_circle->center();
        *m_circle = Circle<Inexact>(c, CGAL::squared_distance(c, to));
    }
}

void GeometryWidget::CircleEditable::endDrag() {}

bool GeometryWidget::CircleEditable::isCloseToCenter(Point<Inexact> location, Number<Inexact> radius) const {
    return (m_circle->center() - location).squared_length() < radius * radius;
}

bool GeometryWidget::CircleEditable::isCloseToBoundary(Point<Inexact> location, Number<Inexact> radius) const {
    auto d = sqrt(CGAL::squared_distance(m_circle->center(), location));
    auto r = sqrt(m_circle->squared_radius());
    return abs(d - r) < radius;
}

GeometryWidget::GeometryWidget() {
	setMouseTracking(true);
	m_transform.scale(1, -1);

	m_layerList = new QListWidget(this);
	updateLayerList();
	connect(m_layerList, &QListWidget::itemChanged, [&]() {
		for (int i = 0; i < m_layerList->count(); i++) {
			m_paintings[i].visible = m_layerList->item(i)->checkState() == Qt::Checked;
			if (m_paintings[i].visible) {
				m_invisibleLayerNames.erase(m_paintings[i].name);
			} else {
				m_invisibleLayerNames.insert(m_paintings[i].name);
			}
		}
		update();
	});

	m_zoomBar = new QToolBar(this);
	m_zoomOutButton = new QToolButton(m_zoomBar);
	m_zoomOutButton->setText("-");
	connect(m_zoomOutButton, &QToolButton::clicked, this, &GeometryWidget::zoomOut);
	m_zoomBar->addWidget(m_zoomOutButton);
	m_zoomSlider = new QSlider(m_zoomBar);
	m_zoomSlider->setOrientation(Qt::Horizontal);
	m_zoomSlider->setMinimumWidth(200);
	m_zoomSlider->setMaximumWidth(200);
	m_zoomSlider->setMinimum(0);
	m_zoomSlider->setMaximum(200);
	m_zoomSlider->setEnabled(false);
	m_zoomBar->addWidget(m_zoomSlider);
	m_zoomInButton = new QToolButton(m_zoomBar);
	m_zoomInButton->setText("+");
	connect(m_zoomInButton, &QToolButton::clicked, this, &GeometryWidget::zoomIn);
	m_zoomBar->addWidget(m_zoomInButton);
	m_zoomBar->addSeparator();
	m_saveToIpeButton = new QToolButton(m_zoomBar);
	m_saveToIpeButton->setText("Save as Ipe");
	connect(m_saveToIpeButton, &QToolButton::clicked, this, &GeometryWidget::saveToIpe);
	m_zoomBar->addWidget(m_saveToIpeButton);
	m_saveToSvgButton = new QToolButton(m_zoomBar);
	m_saveToSvgButton->setText("Save as SVG");
	connect(m_saveToSvgButton, &QToolButton::clicked, this, &GeometryWidget::saveToSvg);
	m_zoomBar->addWidget(m_saveToSvgButton);
}

GeometryWidget::GeometryWidget(std::shared_ptr<GeometryPainting> painting) : GeometryWidget() {
	m_paintings.push_back(DrawnPainting{painting, "", true});
}

void GeometryWidget::resizeEvent(QResizeEvent* e) {
	QWidget::resizeEvent(e);
	QRect r = rect();
	QSize zoomBarSize = m_zoomBar->sizeHint();
	QRect zoomBarRect(QPoint{r.left(), r.bottom() - zoomBarSize.height()},
	                  QPoint{r.left() + zoomBarSize.width(), r.bottom()});
	m_zoomBar->setGeometry(zoomBarRect);
}

void GeometryWidget::paintEvent(QPaintEvent* event) {
	m_painter = std::make_unique<QPainter>(this);
	m_painter->setRenderHint(QPainter::Antialiasing);
	m_painter->fillRect(rect(), Qt::white);
	if (m_drawAxes) {
		drawAxes();
	}
	for (const auto& painting : m_paintings) {
		if (painting.visible) {
			pushStyle();
			painting.m_painting->paint(*this);
			popStyle();
		}
	}

	Point<Inexact> mouseLocation = inverseConvertPoint(m_mousePos);
	for (const auto& editable : m_editables) {
		if (editable->drawHoverHint(mouseLocation, 10.0f / zoomFactor())) {
			break;
		}
	}

	drawCoordinates();
}

void GeometryWidget::mouseMoveEvent(QMouseEvent* event) {
	m_mousePos = event->pos();

	if (m_panning) {
		QPointF delta = m_mousePos - m_previousMousePos;
		QTransform translation;
		translation.translate(delta.x(), delta.y());
		m_transform = m_transform * translation;
	} else if (m_mouseButtonDown) {
		if (m_dragging) {
			emit dragMoved(inverseConvertPoint(m_mousePos));
		} else if (m_activeEditable) {
			m_activeEditable->handleDrag(inverseConvertPoint(m_mousePos));
			emit edited();
		} else {
			m_dragging = true;
			emit dragStarted(inverseConvertPoint(m_previousMousePos));
			emit dragMoved(inverseConvertPoint(m_mousePos));
		}
	}
	m_previousMousePos = m_mousePos;

	update();
}

void GeometryWidget::mousePressEvent(QMouseEvent* event) {
	m_mouseButtonDown = true;
	if ((event->button() & Qt::RightButton) ||
	    QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier)) {
		// initiate canvas panning when dragging with the right mouse button
		// or when holding Ctrl while dragging
		m_panning = true;
		setCursor(Qt::ClosedHandCursor);
		update();

	} else {
		// else, see if there is some editable that wants to respond
		Point<Inexact> mouseLocation = inverseConvertPoint(m_mousePos);
		for (auto& editable : m_editables) {
			if (editable->startDrag(mouseLocation, 10.0f / zoomFactor())) {
				m_activeEditable = editable.get();
				break;
			}
		}
	}
	m_previousMousePos = event->pos();
}

void GeometryWidget::mouseReleaseEvent(QMouseEvent* event) {
	m_mouseButtonDown = false;
	if (m_panning) {
		m_panning = false;
		setCursor(Qt::ArrowCursor);
		update();
	} else if (m_dragging) {
		m_dragging = false;
		emit dragEnded(inverseConvertPoint(m_previousMousePos));
	} else if (m_activeEditable) {
		m_activeEditable = nullptr;
	} else {
		emit clicked(inverseConvertPoint(m_previousMousePos));
	}
}

void GeometryWidget::leaveEvent(QEvent* event) {
	// TODO?
	//update();
}

QSize GeometryWidget::sizeHint() const {
	return QSize(800, 450);
}

void GeometryWidget::wheelEvent(QWheelEvent* event) {
	if (event->angleDelta().isNull()) {
		return;
	}

	double delta = event->angleDelta().y();
	double factor = pow(2, delta / 240);

	// limit the zoom factor
	if (factor * m_transform.m11() > m_maxZoom) {
		factor = m_maxZoom / m_transform.m11();
	} else if (factor * m_transform.m11() < m_minZoom) {
		factor = m_minZoom / m_transform.m11();
	}

	QPointF mousePos = event->position();
	mousePos -= QPointF(width() / 2.0, height() / 2.0);
	QTransform transform;
	transform.translate(mousePos.x(), mousePos.y());
	transform.scale(factor, factor);
	transform.translate(-mousePos.x(), -mousePos.y());
	m_transform *= transform;

	updateZoomSlider();
	update();
}

QPointF GeometryWidget::convertPoint(Point<Inexact> p) const {
	QPointF mapped = m_transform.map(QPointF(p.x() + 0.5, p.y() + 0.5));
	return mapped + QPointF(width(), height()) / 2.0;
}

QRectF GeometryWidget::convertBox(Box b) const {
	QPointF topLeft = convertPoint(Point<Inexact>(b.xmin(), b.ymin()));
	QPointF bottomRight = convertPoint(Point<Inexact>(b.xmax(), b.ymax()));
	return QRectF(topLeft, bottomRight);
}

Point<Inexact> GeometryWidget::inverseConvertPoint(QPointF p) const {
	QPointF toMap = p - QPointF(width(), height()) / 2.0;
	QPointF transformed = m_transform.inverted().map(toMap) - QPointF(0.5, 0.5);
	return Point<Inexact>(transformed.x(), transformed.y());
}

Box GeometryWidget::inverseConvertBox(QRectF r) const {
	Point<Inexact> bottomLeft = inverseConvertPoint(r.bottomLeft());
	Point<Inexact> topRight = inverseConvertPoint(r.topRight());
	return Box(bottomLeft.x(), bottomLeft.y(), topRight.x(), topRight.y());
}

void GeometryWidget::drawAxes() {
	Box bounds = inverseConvertBox(rect());
	pushStyle();

	double tickScale = log10(m_transform.m11());
	double majorScale = pow(10, 2 - floor(tickScale));

	setMode(DrawMode::stroke);
	if (m_gridMode == GridMode::CARTESIAN) {
		// minor grid lines
		double minorTint = tickScale - floor(tickScale);
		int minorColor = 255 - 64 * minorTint;
		setStroke(Color{minorColor, minorColor, minorColor}, 1);
		for (int i = floor(bounds.xmin() / (majorScale / 10));
		     i <= bounds.xmax() / (majorScale / 10); ++i) {
			GeometryRenderer::draw(
			    Segment<Inexact>(Point<Inexact>(i * majorScale / 10, bounds.ymin()),
			                     Point<Inexact>(i * majorScale / 10, bounds.ymax())));
		}
		for (int i = floor(bounds.ymin() / (majorScale / 10));
		     i <= bounds.ymax() / (majorScale / 10); ++i) {
			GeometryRenderer::draw(
			    Segment<Inexact>(Point<Inexact>(bounds.xmin(), i * majorScale / 10),
			                     Point<Inexact>(bounds.xmax(), i * majorScale / 10)));
		}

		// major grid lines
		setStroke(Color{192, 192, 192}, 1);
		for (int i = floor(bounds.xmin() / majorScale); i <= bounds.xmax() / majorScale; ++i) {
			GeometryRenderer::draw(Segment<Inexact>(Point<Inexact>(i * majorScale, bounds.ymin()),
			                                        Point<Inexact>(i * majorScale, bounds.ymax())));
		}
		for (int i = floor(bounds.ymin() / majorScale); i <= bounds.ymax() / majorScale; ++i) {
			GeometryRenderer::draw(Segment<Inexact>(Point<Inexact>(bounds.xmin(), i * majorScale),
			                                        Point<Inexact>(bounds.xmax(), i * majorScale)));
		}

	} else if (m_gridMode == GridMode::POLAR) {
		Number<Inexact> minRadius = std::numeric_limits<Number<Inexact>>::infinity();
		Number<Inexact> maxRadius = 0;
		const std::vector<Point<Inexact>> candidates = {
		    {bounds.xmin(), bounds.ymax()},
		    {bounds.xmin(), 0},
		    {0, bounds.ymax()},
		    {bounds.xmin(), bounds.ymin()},
		    {bounds.xmax(), bounds.ymax()},
		    {bounds.xmax(), 0},
		    {0, bounds.ymin()},
		    {bounds.xmax(), bounds.ymin()},
		    {0, 0}
		};
		for (const auto& c : candidates) {
			if (c.x() >= bounds.xmin() && c.x() <= bounds.xmax() && c.y() >= bounds.ymin() &&
			    c.y() <= bounds.ymax()) {
				Number<Inexact> r = std::hypot(c.x(), c.y());
				minRadius = std::min(minRadius, r);
				maxRadius = std::max(maxRadius, r);
			}
		}
		// minor grid lines
		double minorTint = tickScale - floor(tickScale);
		int minorColor = 255 - 64 * minorTint;
		setStroke(Color{minorColor, minorColor, minorColor}, 1);
		for (int i = minRadius / (majorScale / 10); i <= maxRadius / (majorScale / 10); ++i) {
			draw(Circle<Inexact>(CGAL::ORIGIN, std::pow(i * majorScale / 10, 2)));
		}
		// major grid lines
		setStroke(Color{192, 192, 192}, 1);
		for (int i = minRadius / majorScale; i <= maxRadius / majorScale; ++i) {
			draw(Circle<Inexact>(CGAL::ORIGIN, std::pow(i * majorScale, 2)));
		}
	}

	// axes
	setStroke(Color{150, 150, 150}, 1.8);
	GeometryRenderer::draw(
	    Segment<Inexact>(Point<Inexact>(bounds.xmin(), 0), Point<Inexact>(bounds.xmax(), 0)));
	GeometryRenderer::draw(
	    Segment<Inexact>(Point<Inexact>(0, bounds.ymin()), Point<Inexact>(0, bounds.ymax())));

	// labels
	QPointF origin = convertPoint(Point<Inexact>(0, 0));
	m_painter->drawText(QRectF(origin + QPointF{-100, 5}, origin + QPointF{-5, 100}),
	                    Qt::AlignRight, "0");

	for (int i = floor(bounds.xmin() / majorScale); i <= bounds.xmax() / majorScale + 1; ++i) {
		if (i != 0) {
			origin = convertPoint(Point<Inexact>(i * majorScale, 0));
			if (m_gridMode == GridMode::CARTESIAN) {
				if (origin.y() < 0) {
					origin.setY(0);
				} else if (origin.y() > rect().bottom() - 30) {
					origin.setY(rect().bottom() - 30);
				}
			}
			m_painter->drawText(QRectF(origin + QPointF{-100, 5}, origin + QPointF{-5, 100}),
			                    Qt::AlignRight, QString::number(i * majorScale));
		}
	}
	QFontMetricsF metrics(m_painter->font());
	for (int i = floor(bounds.ymin() / majorScale); i <= bounds.ymax() / majorScale + 1; ++i) {
		if (i != 0) {
			origin = convertPoint(Point<Inexact>(0, i * majorScale));
			QString label = QString::number(i * majorScale);
			double length = metrics.width(label);
			if (m_gridMode == GridMode::CARTESIAN) {
				if (origin.x() < length + 10) {
					origin.setX(length + 10);
				} else if (origin.x() > rect().right() - 0) {
					origin.setX(rect().right() - 0);
				}
			}
			m_painter->drawText(QRectF(origin + QPointF{-100, 5}, origin + QPointF{-5, 100}),
			                    Qt::AlignRight, label);
		}
	}

	popStyle();
}

void GeometryWidget::drawCoordinates() {
	Point<Inexact> converted = inverseConvertPoint(m_mousePos);
	m_painter->setPen(QPen(QColor(0, 0, 0)));
	QString coordinate;
	if (m_gridMode == GridMode::CARTESIAN) {
		double decimalCount = std::max(0, static_cast<int>(log10(m_transform.m11())));
		coordinate = "(" + QString::number(converted.x(), 'f', decimalCount) + ", " +
		             QString::number(converted.y(), 'f', decimalCount) + ")";
	} else if (m_gridMode == GridMode::POLAR) {
		double rDecimalCount = std::max(0, static_cast<int>(log10(m_transform.m11())));
		Number<Inexact> r = std::hypot(converted.x(), converted.y());
		double phiDecimalCount = std::max(0, static_cast<int>(log10(m_transform.m11() * r)) + 1);
		Number<Inexact> theta = std::atan2(converted.y(), converted.x());
		coordinate = "(r = " + QString::number(r, 'f', rDecimalCount) + ", φ = " +
		             QString::number(theta / M_PI, 'f', phiDecimalCount) + "π)";
	}
	m_painter->drawText(rect().marginsRemoved(QMargins(10, 10, 10, 10)),
	                    Qt::AlignRight | Qt::AlignBottom, coordinate);
	m_painter->end();
}

void GeometryWidget::updateZoomSlider() {
	double zoom = m_transform.m11();
	double fraction = log(zoom / m_minZoom) / log(m_maxZoom / m_minZoom);
	m_zoomSlider->setValue(fraction * 200);
}

void GeometryWidget::updateLayerList() {
	if (m_paintings.size() < 2) {
		m_layerList->hide();
		return;
	}
	m_layerList->show();
	m_layerList->clear();
	for (auto painting : m_paintings) {
		QListWidgetItem* item =
		    new QListWidgetItem(QString::fromStdString(painting.name), m_layerList);
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		item->setCheckState(painting.visible ? Qt::Checked : Qt::Unchecked);
	}
}

void GeometryWidget::draw(const Point<Inexact>& p) {
	m_painter->setPen(Qt::NoPen);
	m_painter->setBrush(m_style.m_strokeColor);
	QPointF p2 = convertPoint(p);
	m_painter->drawEllipse(QRectF(p2.x() - 0.5 * m_style.m_pointSize,
	                              p2.y() - 0.5 * m_style.m_pointSize, m_style.m_pointSize,
	                              m_style.m_pointSize));
}

void GeometryWidget::addPolygonToPath(QPainterPath& path, const Polygon<Inexact>& p) {
	for (auto vertex = p.vertices_begin(); vertex != p.vertices_end(); vertex++) {
		if (vertex == p.vertices_begin()) {
			path.moveTo(convertPoint(*vertex));
		} else {
			path.lineTo(convertPoint(*vertex));
		}
	}
	path.closeSubpath();
}

void GeometryWidget::draw(const Circle<Inexact>& c) {
	setupPainter();
	QRectF rect = convertBox(c.bbox());
	m_painter->drawEllipse(rect);
}

void GeometryWidget::draw(const CubicBezierSpline& s) {
	if (s.empty()) return;
	setupPainter();
	QPainterPath path;
	path.moveTo(convertPoint(s.source()));
	for (int i = 0; i < s.numCurves(); ++i) {
		path.cubicTo(convertPoint(s.controlPoint(3 * i + 1)), convertPoint(s.controlPoint(3 * i + 2)),
		             convertPoint(s.controlPoint(3 * i + 3)));
	}
	m_painter->drawPath(path);
	if (m_style.m_mode & vertices) {
		for (int i = 0; i <= s.numCurves(); ++i) {
			draw(s.controlPoint(3 * i));
		}
	}
}

void GeometryWidget::draw(const Ray<Inexact>& r) {
	Box bounds = inverseConvertBox(rect());
	auto result = intersection(r, Rectangle<Inexact>(Point<Inexact>(bounds.xmin(), bounds.ymin()), Point<Inexact>(bounds.xmax(), bounds.ymax())));
	if (result) {
		if (std::holds_alternative<Segment<Inexact>>(*result)) {
			const Segment<Inexact> s = std::get<Segment<Inexact>>(*result);
			int oldMode = m_style.m_mode;
			setMode(oldMode & ~vertices);
			GeometryRenderer::draw(s);
			setMode(oldMode);
		}
		if (m_style.m_mode & vertices) {
			draw(r.source());
		}
	}
}

void GeometryWidget::draw(const Line<Inexact>& l) {
	Box bounds = inverseConvertBox(rect());
	auto result =
	    intersection(l, CGAL::Iso_rectangle_2<Inexact>(Point<Inexact>(bounds.xmin(), bounds.ymin()),
	                                                   Point<Inexact>(bounds.xmax(), bounds.ymax())));
	if (result) {
		if (std::holds_alternative<Segment<Inexact>>(*result)) {
			const Segment<Inexact> s = std::get<Segment<Inexact>>(*result);
			int oldMode = m_style.m_mode;
			setMode(oldMode & ~vertices);
			GeometryRenderer::draw(s);
			setMode(oldMode);
		}
	}
}

void GeometryWidget::draw(const Halfplane<Inexact>& h) {
	auto l = h.line();
	Box bounds = inverseConvertBox(rect());
	auto result =
		intersection(l, CGAL::Iso_rectangle_2<Inexact>(Point<Inexact>(bounds.xmin(), bounds.ymin()),
													   Point<Inexact>(bounds.xmax(), bounds.ymax())));
	if (result) {
		if (std::holds_alternative<Segment<Inexact>>(*result)) {
			const Segment<Inexact> s = std::get<Segment<Inexact>>(*result);
			int oldMode = m_style.m_mode;
			if (oldMode & fill) {
				// Draw filled half-plane
				setMode(fill);
				Rectangle<Inexact> rect(bounds.xmin(), bounds.ymin(), bounds.xmax(), bounds.ymax());
				Polygon<Inexact> poly = h.polygon(rect);
				GeometryRenderer::draw(poly);
			}
			setMode(oldMode & ~vertices);
			GeometryRenderer::draw(s);
			setMode(oldMode);
		}
	}
}

QPainterPath GeometryWidget::renderPathToQt(const RenderPath& p) {
    QPainterPath path;
    for (RenderPath::Command c : p.commands()) {
        if (std::holds_alternative<RenderPath::MoveTo>(c)) {
            Point<Inexact> to = std::get<RenderPath::MoveTo>(c).m_to;
            path.moveTo(convertPoint(to));

        } else if (std::holds_alternative<RenderPath::LineTo>(c)) {
            Point<Inexact> to = std::get<RenderPath::LineTo>(c).m_to;
            path.lineTo(convertPoint(std::get<RenderPath::LineTo>(c).m_to));

        } else if (std::holds_alternative<RenderPath::ArcTo>(c)) {
            Point<Inexact> from = inverseConvertPoint(path.currentPosition());
            Point<Inexact> center = std::get<RenderPath::ArcTo>(c).m_center;
            Point<Inexact> to = std::get<RenderPath::ArcTo>(c).m_to;
            bool clockwise = std::get<RenderPath::ArcTo>(c).m_clockwise;

            double radius = sqrt((center - to).squared_length());
            Vector<Inexact> diagonal(radius, radius);
            QRectF bounds(convertPoint(center - diagonal), convertPoint(center + diagonal));

            double startAngle = atan2((from - center).y(), (from - center).x()) * (180 / M_PI);
            double endAngle = atan2((to - center).y(), (to - center).x()) * (180 / M_PI);
            double sweepLength = endAngle - startAngle;
            if (!clockwise && sweepLength < 0) {
                sweepLength += 360;  // counter-clockwise -> positive sweepLength
            }
            if (clockwise && sweepLength > 0) {
                sweepLength -= 360;  // clockwise -> negative sweepLength
            }
            // the angles are negative because the y-axis is pointing up here
            // instead of down
            path.arcTo(bounds, -startAngle, -sweepLength);

        } else if (std::holds_alternative<RenderPath::Close>(c)) {
            path.closeSubpath();
        }
    }

    return path;
}

void GeometryWidget::draw(const RenderPath& p) {
	setupPainter();
	QPainterPath path = renderPathToQt(p);
	std::vector<Point<Inexact>> verticesToDraw;
    p.vertices(std::back_inserter(verticesToDraw));
	m_painter->drawPath(path);
	if (m_style.m_mode & vertices) {
		for (const Point<Inexact>& vertex : verticesToDraw) {
			draw(vertex);
		}
	}
}

void GeometryWidget::drawText(const Point<Inexact>& p, const std::string& text, bool escape) {
	setupPainter();
	QPointF p2 = convertPoint(p);
	m_painter->drawText(QRectF(p2 - QPointF{500, 250}, p2 + QPointF{500, 250}), m_textAlignment,
	                    QString::fromStdString(text));
}

void GeometryWidget::setupPainter() {
	if (m_style.m_mode & GeometryRenderer::fill) {
		m_painter->setBrush(QBrush(m_style.m_fillColor));
	} else {
		m_painter->setBrush(Qt::NoBrush);
	}
	if (m_style.m_mode & GeometryRenderer::stroke) {
		m_painter->setPen(QPen(m_style.m_strokeColor,
		                       m_style.m_strokeWidth * (m_style.m_absoluteWidth ? zoomFactor() : 1),
		                       Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin));
	} else {
		m_painter->setPen(Qt::NoPen);
	}
}

void GeometryWidget::pushStyle() {
	m_styleStack.push(m_style);
}

void GeometryWidget::popStyle() {
	m_style = m_styleStack.top();
	m_styleStack.pop();
}

void GeometryWidget::setMode(int mode) {
	m_style.m_mode = mode;
}

void GeometryWidget::setStroke(Color color, double width, bool absoluteWidth) {
	m_style.m_strokeColor = QColor(color.r, color.g, color.b);
	m_style.m_strokeWidth = width;
	m_style.m_absoluteWidth = absoluteWidth;
}

void GeometryWidget::setStrokeOpacity(int alpha) {
	m_style.m_strokeColor.setAlpha(alpha);
}

void GeometryWidget::setFill(Color color) {
	m_style.m_fillColor.setRgb(color.r, color.g, color.b, m_style.m_fillColor.alpha());
}

void GeometryWidget::setFillOpacity(int alpha) {
	m_style.m_fillColor.setAlpha(alpha);
}

void GeometryWidget::setClipPath(const RenderPath& clipPath) {
    QPainterPath qtClipPath = renderPathToQt(clipPath);
	auto hasClipping = m_painter->hasClipping();
	// Setting the clip path automatically enables clipping.
    m_painter->setClipPath(qtClipPath);
	m_painter->setClipping(hasClipping);
}

void GeometryWidget::setClipping(bool enable) {
    m_painter->setClipping(enable);
}

void GeometryWidget::setLineCap(LineCap lineCap) {
	auto pen = m_painter->pen();
	switch(lineCap) {
	case RoundCap: {
		pen.setCapStyle(Qt::RoundCap);
		break;
	}
	case ButtCap: {
		pen.setCapStyle(Qt::FlatCap);
		break;
	}
	case SquareCap: {
		pen.setCapStyle(Qt::SquareCap);
		break;
	}
	}
	m_painter->setPen(pen);
}

void GeometryWidget::setLineJoin(LineJoin lineJoin) {
	auto pen = m_painter->pen();
	switch(lineJoin) {
	case RoundJoin: {
		pen.setJoinStyle(Qt::RoundJoin);
		break;
	}
	case MiterJoin: {
		pen.setJoinStyle(Qt::MiterJoin);
		break;
	}
	case BevelJoin: {
		pen.setJoinStyle(Qt::BevelJoin);
		break;
	}
	}
	m_painter->setPen(pen);
}

void GeometryWidget::setHorizontalTextAlignment(HorizontalTextAlignment alignment) {
	switch(alignment) {
	case AlignHCenter: {
		m_textAlignment = (m_textAlignment & Qt::AlignVertical_Mask) | Qt::AlignHCenter;
		break;
	}
	case AlignLeft: {
		m_textAlignment = (m_textAlignment & Qt::AlignVertical_Mask) | Qt::AlignLeft;
		break;
	}
	case AlignRight: {
		m_textAlignment = (m_textAlignment & Qt::AlignVertical_Mask) | Qt::AlignRight;
		break;
	}
	}
}

void GeometryWidget::setVerticalTextAlignment(VerticalTextAlignment alignment) {
	switch(alignment) {
	case AlignVCenter: {
		m_textAlignment = (m_textAlignment & Qt::AlignHorizontal_Mask) | Qt::AlignVCenter;
		break;
	}
	case AlignTop: {
		m_textAlignment = (m_textAlignment & Qt::AlignHorizontal_Mask) | Qt::AlignTop;
		break;
	}
	case AlignBottom: {
		m_textAlignment = (m_textAlignment & Qt::AlignHorizontal_Mask) | Qt::AlignBottom;
		break;
	}
	case AlignBaseline: {
		m_textAlignment = (m_textAlignment & Qt::AlignHorizontal_Mask) | Qt::AlignBaseline;
		break;
	}
	}
}

void GeometryWidget::addPainting(std::shared_ptr<GeometryPainting> painting, const std::string& name) {
	bool visible = !m_invisibleLayerNames.contains(name);
	m_paintings.push_back(DrawnPainting{painting, name, visible});
	updateLayerList();
}

void GeometryWidget::addPainting(const std::function<void(renderer::GeometryRenderer&)>& draw_function, const std::string& name) {
	auto painting = std::make_shared<FunctionPainting>(draw_function);
	addPainting(painting, name);
}

void GeometryWidget::clear() {
	m_paintings.clear();
	updateLayerList();
}

Number<Inexact> GeometryWidget::zoomFactor() const {
	return m_transform.m11();
}

void GeometryWidget::registerEditable(std::shared_ptr<Point<Inexact>> point) {
	m_editables.push_back(std::make_unique<PointEditable>(this, point));
}

void GeometryWidget::registerEditable(std::shared_ptr<Circle<Inexact>> circle) {
    m_editables.push_back(std::make_unique<CircleEditable>(this, circle));
}

void GeometryWidget::registerEditable(std::shared_ptr<Polygon<Inexact>> polygon) {
	m_editables.push_back(std::make_unique<PolygonEditable>(this, polygon));
}

void GeometryWidget::setDrawAxes(bool drawAxes) {
	m_drawAxes = drawAxes;
	update();
}

void GeometryWidget::setMinZoom(double minZoom) {
	m_minZoom = minZoom;
}

void GeometryWidget::setMaxZoom(double maxZoom) {
	m_maxZoom = maxZoom;
}

void GeometryWidget::zoomIn() {
	m_transform *= 1.5;
	if (m_transform.m11() > m_maxZoom) {
		m_transform *= m_maxZoom / m_transform.m11();
	}
	updateZoomSlider();
	update();
}

void GeometryWidget::zoomOut() {
	m_transform /= 1.5;
	if (m_transform.m11() < m_minZoom) {
		m_transform *= m_minZoom / m_transform.m11();
	}
	updateZoomSlider();
	update();
}

void GeometryWidget::centerViewOn(Point<Inexact> newCenter) {
	Point<Inexact> currentCenter = inverseConvertPoint(QPointF(width(), height()) / 2.0);
	m_transform.translate(currentCenter.x() - newCenter.x(), currentCenter.y() - newCenter.y());
	update();
}

void GeometryWidget::fitInView(Box bbox) {
	centerViewOn(Point<Inexact>((bbox.xmin() + bbox.xmax()) / 2, (bbox.ymin() + bbox.ymax()) / 2));
	double newZoom = std::min(width() / bbox.x_span(), height() / bbox.y_span());
	if (newZoom < m_minZoom) {
		newZoom = m_minZoom;
	} else if (newZoom > m_maxZoom) {
		newZoom = m_maxZoom;
	}
	m_transform *= newZoom / m_transform.m11();
	updateZoomSlider();
	update();
}

void GeometryWidget::setGridMode(GridMode mode) {
	m_gridMode = mode;
	update();
}

void GeometryWidget::saveToIpe() {
	QString fileName = QFileDialog::getSaveFileName(this, "Save drawing", ".", "Ipe XML files (*.ipe)");
	if (fileName == nullptr) {
		return;
	}

	IpeRenderer renderer;
	for (const DrawnPainting& painting : m_paintings) {
		renderer.addPainting(painting.m_painting, painting.name);
	}
	std::filesystem::path filePath = fileName.toStdString();
	if (!filePath.has_extension()) {
		filePath.replace_extension(".ipe");
	}
	renderer.save(fileName.toStdString());
}

void GeometryWidget::saveToSvg() {
	QString fileName = QFileDialog::getSaveFileName(this, "Save drawing", ".", "SVG files (*.svg)");
	if (fileName == nullptr) {
		return;
	}

	SvgRenderer renderer;
	for (const DrawnPainting& painting : m_paintings) {
		renderer.addPainting(painting.m_painting, painting.name);
	}
	renderer.save(fileName.toStdString());
}

} // namespace cartocrow::renderer
