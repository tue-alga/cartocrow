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
#include <ipereference.h>
#include <ipestyle.h>
#include <ipetext.h>

#include <fstream>
#include <string>

namespace cartocrow::renderer {

IpeRenderer::IpeRenderer(const std::shared_ptr<GeometryPainting>& painting) {
	m_paintings.push_back(painting);
}

void IpeRenderer::save(const std::filesystem::path& file) {
	ipe::Platform::initLib(ipe::IPELIB_VERSION);
	ipe::Document document;
	ipe::Layout layout;
	layout.iOrigin = ipe::Vector(0, 0);
	layout.iPaperSize = ipe::Vector(1000, 1000);
	layout.iFrameSize = ipe::Vector(1000, 1000);

	std::string diskMarkDefinition =
	    "<ipestyle name=\"marks\">\n"
	    "<symbol name=\"mark/disk(sx)\" transformations=\"translations\">\n"
	    "<path fill=\"sym-stroke\">\n"
	    "0.6 0 0 0.6 0 0 e\n"
	    "</path>\n"
	    "</symbol>\n"
	    "</ipestyle>";
	ipe::Buffer styleBuffer(diskMarkDefinition.data(), diskMarkDefinition.size());
	ipe::BufferSource styleSource(styleBuffer);
	ipe::ImlParser styleParser(styleSource);
	ipe::StyleSheet* diskSheet = styleParser.parseStyleSheet();
	document.cascade()->insert(0, diskSheet);

	ipe::StyleSheet* sizeSheet = new ipe::StyleSheet();
	sizeSheet->setName("paper-size");
	sizeSheet->setLayout(layout);
	document.cascade()->insert(1, sizeSheet);

	m_alphaSheet = new ipe::StyleSheet();
	m_alphaSheet->setName("alpha-values");
	document.cascade()->insert(2, m_alphaSheet);
	setFillOpacity(255); // add default alpha to style sheet

	m_page = new ipe::Page();
	for (auto painting : m_paintings) {
		pushStyle();
		m_page->addLayer();
		m_layer = m_page->countLayers() - 1;
		painting->paint(*this);
		popStyle();
	}

	document.push_back(m_page);
	document.save(file.string().c_str(), ipe::FileFormat::Xml, 0);
}

void IpeRenderer::draw(const Point<Inexact>& p) {
	ipe::Vector vector(p.x(), p.y());
	ipe::Attribute name = ipe::Attribute(true, "mark/disk(sx)");
	ipe::Reference* reference = new ipe::Reference(getAttributesForStyle(), name, vector);
	m_page->append(ipe::TSelect::ENotSelected, m_layer, reference);
}

void IpeRenderer::draw(const Segment<Inexact>& s) {
	ipe::Curve* curve = new ipe::Curve();
	curve->appendSegment(ipe::Vector(s.start().x(), s.start().y()),
	                     ipe::Vector(s.end().x(), s.end().y()));
	ipe::Shape* shape = new ipe::Shape();
	shape->appendSubPath(curve);
	ipe::Path* path = new ipe::Path(getAttributesForStyle(), *shape);
	m_page->append(ipe::TSelect::ENotSelected, m_layer, path);

	if (m_style.m_mode & vertices) {
		draw(s.start());
		draw(s.end());
	}
}

void IpeRenderer::draw(const Line<Inexact>& l) {
	// Crop to document size
	auto bounds = CGAL::Iso_rectangle_2<Inexact>(CGAL::ORIGIN, Point<Inexact>(1000.0, 1000.0));
	auto result = intersection(l, bounds);
	if (result) {
		if (const Segment<Inexact>* s = boost::get<Segment<Inexact>>(&*result)) {
			draw(*s);
		}
	}
}

void IpeRenderer::draw(const Ray<Inexact>& r) {
	// Crop to document size
	auto bounds = CGAL::Iso_rectangle_2<Inexact>(CGAL::ORIGIN, Point<Inexact>(1000.0, 1000.0));
	auto result = intersection(r, bounds);
	if (result) {
		if (const Segment<Inexact>* s = boost::get<Segment<Inexact>>(&*result)) {
			draw(*s);
		}
		if (m_style.m_mode & vertices) {
			draw(r.source());
		}
	}
}

void IpeRenderer::draw(const Polyline<Inexact>& p) {
	ipe::Curve* curve = convertPolylineToCurve(p);
	ipe::Shape* shape = new ipe::Shape();
	shape->appendSubPath(curve);
	ipe::Path* path = new ipe::Path(getAttributesForStyle(), *shape);
	m_page->append(ipe::TSelect::ENotSelected, m_layer, path);

	if (m_style.m_mode & vertices) {
		for (auto v = p.vertices_begin(); v != p.vertices_end(); v++) {
			draw(*v);
		}
	}
}

void IpeRenderer::draw(const Polygon<Inexact>& p) {
	ipe::Curve* curve = convertPolygonToCurve(p);
	ipe::Shape* shape = new ipe::Shape();
	shape->appendSubPath(curve);
	ipe::Path* path = new ipe::Path(getAttributesForStyle(), *shape);
	m_page->append(ipe::TSelect::ENotSelected, m_layer, path);

	if (m_style.m_mode & vertices) {
		for (auto v = p.vertices_begin(); v != p.vertices_end(); v++) {
			draw(*v);
		}
	}
}

void IpeRenderer::draw(const PolygonWithHoles<Inexact>& p) {
	ipe::Curve* curve = convertPolygonToCurve(p.outer_boundary());
	ipe::Shape* shape = new ipe::Shape();
	shape->appendSubPath(curve);
	for (auto hole : p.holes()) {
		ipe::Curve* holeCurve = convertPolygonToCurve(hole);
		shape->appendSubPath(holeCurve);
	}
	ipe::Path* path = new ipe::Path(getAttributesForStyle(), *shape);
	m_page->append(ipe::TSelect::ENotSelected, m_layer, path);

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

void IpeRenderer::draw(const Circle<Inexact>& c) {
	double r = sqrt(c.squared_radius());
	ipe::Matrix matrix =
	    ipe::Matrix(ipe::Vector(c.center().x(), c.center().y())) * ipe::Linear(r, 0, 0, r);
	ipe::Ellipse* ellipse = new ipe::Ellipse(matrix);
	ipe::Shape* shape = new ipe::Shape();
	shape->appendSubPath(ellipse);
	ipe::Path* path = new ipe::Path(getAttributesForStyle(), *shape);
	m_page->append(ipe::TSelect::ENotSelected, m_layer, path);
}

void IpeRenderer::draw(const BezierSpline& s) {
	ipe::Curve* curve = new ipe::Curve();
	for (BezierCurve c : s.curves()) {
		std::vector<ipe::Vector> coords;
		coords.emplace_back(c.source().x(), c.source().y());
		coords.emplace_back(c.sourceControl().x(), c.sourceControl().y());
		coords.emplace_back(c.targetControl().x(), c.targetControl().y());
		coords.emplace_back(c.target().x(), c.target().y());
		curve->appendSpline(coords);
	}
	ipe::Shape* shape = new ipe::Shape();
	shape->appendSubPath(curve);
	ipe::Path* path = new ipe::Path(getAttributesForStyle(), *shape);
	m_page->append(ipe::TSelect::ENotSelected, m_layer, path);

	if (m_style.m_mode & vertices) {
		for (BezierCurve c : s.curves()) {
			draw(c.source());
		}
		draw(s.curves().back().target());
	}
}

void IpeRenderer::drawText(const Point<Inexact>& p, const std::string& text) {
	ipe::String labelText = escapeForLaTeX(text).data();
	ipe::Text* label = new ipe::Text(getAttributesForStyle(), labelText,
	                                 ipe::Vector(p.x(), p.y()), ipe::Text::TextType::ELabel);
	label->setHorizontalAlignment(ipe::THorizontalAlignment::EAlignHCenter);
	label->setVerticalAlignment(ipe::TVerticalAlignment::EAlignVCenter);
	m_page->append(ipe::TSelect::ENotSelected, m_layer, label);
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

ipe::Curve* IpeRenderer::convertPolygonToCurve(const Polygon<Inexact>& p) const {
	ipe::Curve* curve = new ipe::Curve();
	for (auto edge = p.edges_begin(); edge != p.edges_end(); edge++) {
		curve->appendSegment(ipe::Vector(edge->start().x(), edge->start().y()),
		                     ipe::Vector(edge->end().x(), edge->end().y()));
	}
	curve->setClosed(true);
	return curve;
}

ipe::Curve* IpeRenderer::convertPolylineToCurve(const Polyline<Inexact>& p) const {
	ipe::Curve* curve = new ipe::Curve();
	for (auto edge = p.edges_begin(); edge != p.edges_end(); edge++) {
		curve->appendSegment(ipe::Vector(edge->start().x(), edge->start().y()),
		                     ipe::Vector(edge->end().x(), edge->end().y()));
	}
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

void IpeRenderer::addPainting(std::shared_ptr<GeometryPainting> painting) {
	m_paintings.push_back(painting);
}

std::string IpeRenderer::escapeForLaTeX(const std::string& text) const {
	std::string result = "";
	result.reserve(text.size());
	for (int i = 0; i < text.size(); i++) {
		switch (text[i]) {
		case '#':
		case '$':
		case '%':
		case '&':
		case '{':
		case '}':
		case '_':
			result.push_back('\\');
			result.push_back(text[i]);
			break;
		case '~':
		case '^':
			result.push_back('\\');
			result.push_back(text[i]);
			result.push_back('{');
			result.push_back('}');
			break;
		case '\\':
			result += "\\textbackslash{}";
			break;
		default:
			result.push_back(text[i]);
		}
	}
	return result;
}

} // namespace cartocrow::renderer
