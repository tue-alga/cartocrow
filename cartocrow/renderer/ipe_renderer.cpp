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
#include <ipeiml.h>
#include <ipetext.h>

#include <fstream>

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

	std::string input;
	std::fstream fin("/usr/local/share/ipe/7.2.24/styles/basic.isy", std::ios_base::in);
	if (!fin) {
		std::cout << "???" << std::endl;
	}
	try {
		if (fin) {
			using Iterator = std::istreambuf_iterator<char>;
			input.assign(Iterator(fin), Iterator());
		}
	} catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	//std::cout << input << std::endl;
	//std::cout << input.size() << std::endl;
	ipe::Buffer styleBuffer(input.data(), input.size());
	ipe::BufferSource styleSource(styleBuffer);
	ipe::ImlParser styleParser(styleSource);
	ipe::StyleSheet* basicSheet = styleParser.parseStyleSheet();
	if (!basicSheet) {
		std::cout << "oepsie" << std::endl;
	}
	document.cascade()->insert(0, basicSheet);

	ipe::StyleSheet* sheet = new ipe::StyleSheet();
	sheet->setName("paper-size");
	sheet->setLayout(layout);
	document.cascade()->insert(1, sheet);

	m_page = new ipe::Page();
	m_page->addLayer("alpha");

	m_painting.paint(*this);

	std::cout << m_page->count() << std::endl;
	document.push_back(m_page);
	document.save(file.c_str(), ipe::FileFormat::Xml, 0);
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
	/*setupPainter();
	QRectF rect = convertBox(b);
	m_painter->drawRect(rect);*/
}

void IpeRenderer::drawText(const Point& p, const std::string& text) {
	/*setupPainter();
	QPointF p2 = convertPoint(p);
	m_painter->drawText(QRectF(p2 - QPointF{500, 250}, p2 + QPointF{500, 250}), Qt::AlignCenter,
	                    QString::fromStdString(text));*/
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
	return attributes;
}

} // namespace renderer
} // namespace cartocrow
