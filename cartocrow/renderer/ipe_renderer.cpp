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

#include "ipe_renderer.h"

#include "cartocrow/renderer/geometry_renderer.h"
#include <ipeattributes.h>
#include <ipebase.h>
#include <ipedoc.h>
#include <ipegeo.h>
#include <ipeiml.h>
#include <ipestyle.h>
#include <ipetext.h>

#include <fstream>
#include <string>

namespace cartocrow {
namespace renderer {

IpeRenderer::IpeRenderer(GeometryPainting& painting) : m_painting(painting) {}

void IpeRenderer::save(const std::filesystem::path& file) {
	ipe::Platform::initLib(70224);
	ipe::Document document;
	ipe::Layout layout;
	layout.iOrigin = ipe::Vector(0, 0);
	layout.iPaperSize = ipe::Vector(1000, 1000);
	layout.iFrameSize = ipe::Vector(1000, 1000);

	ipe::StyleSheet* sizeSheet = new ipe::StyleSheet();
	sizeSheet->setName("paper-size");
	sizeSheet->setLayout(layout);
	document.cascade()->insert(1, sizeSheet);

	m_alphaSheet = new ipe::StyleSheet();
	m_alphaSheet->setName("alpha-values");
	document.cascade()->insert(2, m_alphaSheet);
	setFillOpacity(255); // add default alpha to style sheet

	m_page = new ipe::Page();
	m_page->addLayer("alpha");

	m_painting.paint(*this);

	std::cout << m_page->count() << std::endl;
	document.push_back(m_page);
	document.save(file.string().c_str(), ipe::FileFormat::Xml, 0);
}

void IpeRenderer::draw(const Point& p) {
	/*m_painter->setPen(Qt::NoPen);
	m_painter->setBrush(m_style.m_strokeColor);
	QPointF p2 = convertPoint(p);
	m_painter->drawEllipse(QRectF(p2.x() - 0.5 * m_style.m_pointSize,
	                              p2.y() - 0.5 * m_style.m_pointSize, m_style.m_pointSize,
	                              m_style.m_pointSize));*/
}

void IpeRenderer::draw(const Segment& s) {
	ipe::Curve* curve = new ipe::Curve();
	curve->appendSegment(ipe::Vector(s.start().x(), s.start().y()),
	                     ipe::Vector(s.end().x(), s.end().y()));
	ipe::Shape* shape = new ipe::Shape();
	shape->appendSubPath(curve);
	ipe::Path* path = new ipe::Path(getAttributesForStyle(), *shape);
	m_page->append(ipe::TSelect::ENotSelected, 0, path);
}

void IpeRenderer::draw(const Polygon& p) {
	ipe::Curve* curve = convertPolygonToCurve(p);
	ipe::Shape* shape = new ipe::Shape();
	shape->appendSubPath(curve);
	ipe::Path* path = new ipe::Path(getAttributesForStyle(), *shape);
	m_page->append(ipe::TSelect::ENotSelected, 0, path);
}

void IpeRenderer::draw(const Polygon_with_holes& p) {
	ipe::Curve* curve = convertPolygonToCurve(p.outer_boundary());
	ipe::Shape* shape = new ipe::Shape();
	shape->appendSubPath(curve);
	for (auto hole : p.holes()) {
		ipe::Curve* holeCurve = convertPolygonToCurve(hole);
		shape->appendSubPath(holeCurve);
	}
	ipe::Path* path = new ipe::Path(getAttributesForStyle(), *shape);
	m_page->append(ipe::TSelect::ENotSelected, 0, path);
}

void IpeRenderer::draw(const Circle& c) {
	double r = sqrt(c.squared_radius());
	ipe::Matrix matrix =
	    ipe::Matrix(ipe::Vector(c.center().x(), c.center().y())) * ipe::Linear(r, 0, 0, r);
	ipe::Ellipse* ellipse = new ipe::Ellipse(matrix);
	ipe::Shape* shape = new ipe::Shape();
	shape->appendSubPath(ellipse);
	ipe::Path* path = new ipe::Path(getAttributesForStyle(), *shape);
	m_page->append(ipe::TSelect::ENotSelected, 0, path);
}

void IpeRenderer::draw(const Box& b) {
	ipe::Curve* curve = new ipe::Curve();
	curve->appendSegment(ipe::Vector(b.xmin(), b.ymin()), ipe::Vector(b.xmax(), b.ymin()));
	curve->appendSegment(ipe::Vector(b.xmax(), b.ymin()), ipe::Vector(b.xmax(), b.ymax()));
	curve->appendSegment(ipe::Vector(b.xmax(), b.ymax()), ipe::Vector(b.xmin(), b.ymax()));
	curve->setClosed(true);
	ipe::Shape* shape = new ipe::Shape();
	shape->appendSubPath(curve);
	ipe::Path* path = new ipe::Path(getAttributesForStyle(), *shape);
	m_page->append(ipe::TSelect::ENotSelected, 0, path);
}

void IpeRenderer::draw(const BezierSpline& s) {
	ipe::Curve* curve = new ipe::Curve();
	for (BezierCurve c : s.curves()) {
		std::vector<ipe::Vector> coords;
		coords.emplace_back(c.source().x(), c.source().y());
		coords.emplace_back(c.source_control().x(), c.source_control().y());
		coords.emplace_back(c.target_control().x(), c.target_control().y());
		coords.emplace_back(c.target().x(), c.target().y());
		curve->appendSpline(coords);
	}
	ipe::Shape* shape = new ipe::Shape();
	shape->appendSubPath(curve);
	ipe::Path* path = new ipe::Path(getAttributesForStyle(), *shape);
	m_page->append(ipe::TSelect::ENotSelected, 0, path);
}

void IpeRenderer::drawText(const Point& p, const std::string& text) {
	ipe::Text* label = new ipe::Text(getAttributesForStyle(), text.data(),
	                                 ipe::Vector(p.x(), p.y()), ipe::Text::TextType::ELabel);
	label->setHorizontalAlignment(ipe::THorizontalAlignment::EAlignHCenter);
	label->setVerticalAlignment(ipe::TVerticalAlignment::EAlignVCenter);
	m_page->append(ipe::TSelect::ENotSelected, 0, label);
}

void IpeRenderer::pushStyle() {
	m_styleStack.push(m_style);
}

void IpeRenderer::popStyle() {
	m_style = m_styleStack.top();
	m_styleStack.pop();
}

void IpeRenderer::setMode(int mode) {
	m_style.m_mode = mode;
}

void IpeRenderer::setStroke(Color color, double width) {
	const double factor = 1000.0 / 255.0;
	m_style.m_strokeColor = ipe::Color(color.r * factor, color.g * factor, color.b * factor);
	m_style.m_strokeWidth = width;
}

void IpeRenderer::setFill(Color color) {
	const double factor = 1000.0 / 255.0;
	m_style.m_fillColor = ipe::Color(color.r * factor, color.g * factor, color.b * factor);
}

void IpeRenderer::setFillOpacity(int alpha) {
	// Ipe does not allow arbitrary opacity values; it only allows symbolic
	// references to alpha values from the stylesheet.
	// Therefore, we check if the requested opacity value already exists. If
	// not, we add it to the stylesheet.
	ipe::Attribute name = ipe::Attribute(true, std::to_string(alpha).data());
	if (!m_alphaSheet->has(ipe::Kind::EOpacity, name)) {
		m_alphaSheet->add(ipe::Kind::EOpacity, name,
		                  ipe::Attribute(ipe::Fixed::fromDouble(alpha / 255.0)));
	}
	m_style.m_fillOpacity = name;
}

ipe::Curve* IpeRenderer::convertPolygonToCurve(const Polygon& p) const {
	ipe::Curve* curve = new ipe::Curve();
	for (auto edge = p.edges_begin(); edge != p.edges_end(); edge++) {
		curve->appendSegment(ipe::Vector(edge->start().x(), edge->start().y()),
		                     ipe::Vector(edge->end().x(), edge->end().y()));
	}
	curve->setClosed(true);
	return curve;
}

ipe::AllAttributes IpeRenderer::getAttributesForStyle() const {
	ipe::AllAttributes attributes;
	if ((m_style.m_mode & GeometryRenderer::fill) && (m_style.m_mode & GeometryRenderer::stroke)) {
		attributes.iPathMode = ipe::TPathMode::EStrokedAndFilled;
	} else if (m_style.m_mode & GeometryRenderer::fill) {
		attributes.iPathMode = ipe::TPathMode::EFilledOnly;
	} else {
		attributes.iPathMode = ipe::TPathMode::EStrokedOnly;
	}
	attributes.iPen = ipe::Attribute(ipe::Fixed::fromDouble(m_style.m_strokeWidth));
	attributes.iStroke = ipe::Attribute(ipe::Color(m_style.m_strokeColor));
	attributes.iFill = ipe::Attribute(ipe::Color(m_style.m_fillColor));
	attributes.iOpacity = m_style.m_fillOpacity;
	return attributes;
}

} // namespace renderer
} // namespace cartocrow
