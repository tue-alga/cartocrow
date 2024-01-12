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
	for (auto painting : m_paintings) {
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
	Point<Inexact> topLeft = inverseConvertPoint(r.topLeft());
	Point<Inexact> bottomRight = inverseConvertPoint(r.bottomRight());
	return Box(topLeft.x(), topLeft.y(), bottomRight.x(), bottomRight.y());
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
		for (int i = floor(bounds.xmin() / (majorScale / 10)); i <= bounds.xmax() / (majorScale / 10);
			++i) {
			draw(Segment<Inexact>(Point<Inexact>(i * majorScale / 10, bounds.ymin()),
								Point<Inexact>(i * majorScale / 10, bounds.ymax())));
		}
		for (int i = floor(bounds.ymax() / (majorScale / 10)); i <= bounds.ymin() / (majorScale / 10);
			++i) {
			draw(Segment<Inexact>(Point<Inexact>(bounds.xmin(), i * majorScale / 10),
								Point<Inexact>(bounds.xmax(), i * majorScale / 10)));
		}

		// major grid lines
		setStroke(Color{192, 192, 192}, 1);
		for (int i = floor(bounds.xmin() / majorScale); i <= bounds.xmax() / majorScale; ++i) {
			draw(Segment<Inexact>(Point<Inexact>(i * majorScale, bounds.ymin()),
								Point<Inexact>(i * majorScale, bounds.ymax())));
		}
		for (int i = floor(bounds.ymax() / majorScale); i <= bounds.ymin() / majorScale; ++i) {
			draw(Segment<Inexact>(Point<Inexact>(bounds.xmin(), i * majorScale),
								Point<Inexact>(bounds.xmax(), i * majorScale)));
		}

	} else if (m_gridMode == GridMode::POLAR) {
		Number<Inexact> minRadius = std::numeric_limits<Number<Inexact>>::infinity();
		Number<Inexact> maxRadius = 0;
		const std::vector<Point<Inexact>> candidates = {
		    {bounds.xmin(), bounds.ymin()},
		    {bounds.xmin(), 0},
		    {0, bounds.ymin()},
		    {bounds.xmin(), bounds.ymax()},
		    {bounds.xmax(), bounds.ymin()},
		    {bounds.xmax(), 0},
		    {0, bounds.ymax()},
		    {bounds.xmax(), bounds.ymax()},
		    {0, 0}
		};
		for (const auto& c : candidates) {
			if (c.x() >= bounds.xmin() && c.x() <= bounds.xmax() && c.y() >= bounds.ymax() &&
			    c.y() <= bounds.ymin()) {
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
	draw(Segment<Inexact>(Point<Inexact>(bounds.xmin(), 0), Point<Inexact>(bounds.xmax(), 0)));
	draw(Segment<Inexact>(Point<Inexact>(0, bounds.ymin()), Point<Inexact>(0, bounds.ymax())));

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
	for (int i = floor(bounds.ymax() / majorScale); i <= bounds.ymin() / majorScale + 1; ++i) {
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

void GeometryWidget::draw(const Segment<Inexact>& s) {
	setupPainter();
	QPointF p1 = convertPoint(s.start());
	QPointF p2 = convertPoint(s.end());
	m_painter->drawLine(p1, p2);

	if (m_style.m_mode & vertices) {
		draw(s.start());
		draw(s.end());
	}
}

void GeometryWidget::draw(const Polygon<Inexact>& p) {
	setupPainter();
	QPainterPath path;
	addPolygonToPath(path, p);
	m_painter->drawPath(path);
	if (m_style.m_mode & vertices) {
		for (auto v = p.vertices_begin(); v != p.vertices_end(); v++) {
			draw(*v);
		}
	}
}

void GeometryWidget::draw(const PolygonWithHoles<Inexact>& p) {
	setupPainter();
	QPainterPath path;
	addPolygonToPath(path, p.outer_boundary());
	for (auto hole : p.holes()) {
		addPolygonToPath(path, hole);
	}
	m_painter->drawPath(path);
	if (m_style.m_mode & vertices) {
		for (auto v = p.outer_boundary().vertices_begin(); v != p.outer_boundary().vertices_end(); v++) {
			draw(*v);
		}
		for (auto h = p.holes_begin(); h != p.holes_end(); h++) {
			for (auto v = h->vertices_begin(); v != h->vertices_end(); v++) {
				draw(*v);
			}
		}
	}
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

void GeometryWidget::draw(const BezierSpline& s) {
	setupPainter();
	QPainterPath path;
	path.moveTo(convertPoint(s.curves()[0].source()));
	for (BezierCurve c : s.curves()) {
		path.cubicTo(convertPoint(c.sourceControl()), convertPoint(c.targetControl()),
		             convertPoint(c.target()));
	}
	m_painter->drawPath(path);
	if (m_style.m_mode & vertices) {
		for (BezierCurve c : s.curves()) {
			draw(c.source());
		}
		draw(s.curves().back().target());
	}
}

void GeometryWidget::draw(const Ray<Inexact>& r) {
	Box bounds = inverseConvertBox(rect());
	auto result = intersection(r, CGAL::Iso_rectangle_2<Inexact>(Point<Inexact>(bounds.xmin(), bounds.ymin()), Point<Inexact>(bounds.xmax(), bounds.ymax())));
	if (result) {
		if (const Segment<Inexact>* s = boost::get<Segment<Inexact>>(&*result)) {
			draw(*s);
		}
		if (m_style.m_mode & vertices) {
			draw(r.source());
		}
	}
}

void GeometryWidget::draw(const Line<Inexact>& l) {
	Box bounds = inverseConvertBox(rect());
	auto result = intersection(l, CGAL::Iso_rectangle_2<Inexact>(Point<Inexact>(bounds.xmin(), bounds.ymin()), Point<Inexact>(bounds.xmax(), bounds.ymax())));
	if (result) {
		if (const Segment<Inexact>* s = boost::get<Segment<Inexact>>(&*result)) {
			draw(*s);
		}
	}
}

void GeometryWidget::draw(const Polyline<Inexact>& p) {
	setupPainter();
	QPainterPath path;
	path.moveTo(convertPoint(*p.vertices_begin()));
	for (auto v = p.vertices_begin()++; v != p.vertices_end(); v++) {
		path.lineTo(convertPoint(*v));
	}
	m_painter->drawPath(path);
	if (m_style.m_mode & vertices) {
		for (auto v = p.vertices_begin(); v != p.vertices_end(); v++) {
			draw(*v);
		}
	}
}

void GeometryWidget::drawText(const Point<Inexact>& p, const std::string& text) {
	setupPainter();
	QPointF p2 = convertPoint(p);
	m_painter->drawText(QRectF(p2 - QPointF{500, 250}, p2 + QPointF{500, 250}), Qt::AlignCenter,
	                    QString::fromStdString(text));
}

void GeometryWidget::setupPainter() {
	if (m_style.m_mode & GeometryRenderer::fill) {
		m_painter->setBrush(QBrush(m_style.m_fillColor));
	} else {
		m_painter->setBrush(Qt::NoBrush);
	}
	if (m_style.m_mode & GeometryRenderer::stroke) {
		m_painter->setPen(QPen(m_style.m_strokeColor, m_style.m_strokeWidth, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin));
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

void GeometryWidget::setStroke(Color color, double width) {
	m_style.m_strokeColor = QColor(color.r, color.g, color.b);
	m_style.m_strokeWidth = width;
}

void GeometryWidget::setFill(Color color) {
	m_style.m_fillColor.setRgb(color.r, color.g, color.b, m_style.m_fillColor.alpha());
}

void GeometryWidget::setFillOpacity(int alpha) {
	m_style.m_fillColor.setAlpha(alpha);
}

void GeometryWidget::addPainting(std::shared_ptr<GeometryPainting> painting, const std::string& name) {
	bool visible = !m_invisibleLayerNames.contains(name);
	m_paintings.push_back(DrawnPainting{painting, name, visible});
	updateLayerList();
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
		m_transform /= m_minZoom / m_transform.m11();
	}
	updateZoomSlider();
	update();
}

void GeometryWidget::setGridMode(GridMode mode) {
	m_gridMode = mode;
	update();
}

} // namespace cartocrow::renderer
