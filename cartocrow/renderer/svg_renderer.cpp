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

#include "svg_renderer.h"

#include "geometry_renderer.h"

#include <CGAL/enum.h>
#include <QFileDialog>
#include <QGuiApplication>
#include <QPainterPath>
#include <QPen>
#include <QPoint>
#include <QPolygon>
#include <QSlider>
#include <QToolButton>

#include <cmath>
#include <string>

namespace cartocrow::renderer {

SvgRenderer::SvgRenderer(const std::shared_ptr<GeometryPainting>& painting) {
	m_paintings.push_back(DrawnPainting{painting});
}

SvgRenderer::SvgRenderer(const std::shared_ptr<GeometryPainting>& painting, const std::string& name) {
	m_paintings.push_back(DrawnPainting{painting, name});
}

void SvgRenderer::save(const std::filesystem::path& file) {
	std::locale::global(std::locale("C"));
	m_out.open(file);
	m_out << "<svg version=\"1.1\" xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" xmlns=\"http://www.w3.org/2000/svg\">\n";
	m_out << "<defs><circle id=\"vertex\" cx=\"0\" cy=\"0\" r=\"4\"/></defs>\n";

	for (auto painting : m_paintings) {
		m_out << "<g inkscape:groupmode=\"layer\"";
		if (painting.name) {
			m_out << " inkscape:label=\"" << *painting.name << "\"";
		}
		m_out << ">\n";
		pushStyle();
		painting.m_painting->paint(*this);
		popStyle();
		m_out << "</g>\n";
	}

	m_out << "</svg>\n";
	m_out.close();
}

void SvgRenderer::draw(const Point<Inexact>& p) {
	m_out << "<use xlink:href=\"#vertex\" " << getVertexStyle() << " x=\"" << p.x() << "\" y=\""
	      << -p.y() << "\"/>\n";
}

void SvgRenderer::draw(const Line<Inexact>& l) {
	// crop to document size
	auto bounds = CGAL::Iso_rectangle_2<Inexact>(CGAL::ORIGIN, Point<Inexact>(1000.0, 1000.0));
	auto result = intersection(l, bounds);
	if (result) {
		if (const Segment<Inexact>* s = boost::get<Segment<Inexact>>(&*result)) {
			int oldMode = m_style.m_mode;
			setMode(oldMode & ~vertices);
			GeometryRenderer::draw(*s);
			setMode(oldMode);
		}
	}
}

void SvgRenderer::draw(const Ray<Inexact>& r) {
	// crop to document size
	auto bounds = CGAL::Iso_rectangle_2<Inexact>(CGAL::ORIGIN, Point<Inexact>(1000.0, 1000.0));
	auto result = intersection(r, bounds);
	if (result) {
		if (const Segment<Inexact>* s = boost::get<Segment<Inexact>>(&*result)) {
			int oldMode = m_style.m_mode;
			setMode(oldMode & ~vertices);
			GeometryRenderer::draw(*s);
			setMode(oldMode);
		}
		if (m_style.m_mode & vertices) {
			draw(r.source());
		}
	}
}

void SvgRenderer::draw(const PolygonWithHoles<Inexact>& p) {
	m_out << "<path " << getStyle() << " d=\"";
	m_out << convertPolygonToCurve(p.outer_boundary());
	for (auto hole : p.holes()) {
		m_out << " " << convertPolygonToCurve(hole);
	}
	m_out << "\"/>\n";

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

void SvgRenderer::draw(const Circle<Inexact>& c) {
	double r = sqrt(c.squared_radius());
	m_out << "<circle " << getStyle() << " r=\"" << r << "\" cx=\"" << c.center().x() << "\" cy=\""
	      << -c.center().y() << "\"/>\n";
}

void SvgRenderer::draw(const BezierSpline& s) {
	// TODO
	std::cerr << "The SVG renderer does not support BezierSplines; ignoring\n";
}

void SvgRenderer::draw(const RenderPath& p) {
	m_out << "<path " << getStyle() << " d=\"";
	std::vector<Point<Inexact>> verticesToDraw;
	Point<Inexact> from;
	for (RenderPath::Command c : p.commands()) {
		if (std::holds_alternative<RenderPath::MoveTo>(c)) {
			Point<Inexact> to = std::get<RenderPath::MoveTo>(c).m_to;
			m_out << "M " << to.x() << " " << -to.y() << " ";
			verticesToDraw.push_back(to);
			from = to;

		} else if (std::holds_alternative<RenderPath::LineTo>(c)) {
			Point<Inexact> to = std::get<RenderPath::LineTo>(c).m_to;
			m_out << "L " << to.x() << " " << -to.y() << " ";
			verticesToDraw.push_back(to);
			from = to;

		} else if (std::holds_alternative<RenderPath::ArcTo>(c)) {
			Point<Inexact> center = std::get<RenderPath::ArcTo>(c).m_center;
			Point<Inexact> to = std::get<RenderPath::ArcTo>(c).m_to;
			bool clockwise = std::get<RenderPath::ArcTo>(c).m_clockwise;
			double radius = sqrt((center - to).squared_length());
			bool centerOnLeft = CGAL::orientation(from, to, center) == CGAL::LEFT_TURN;
			double rotation = 0;  // ellipse rotation; irrelevant because we draw circles only
			int largeArc = (centerOnLeft == clockwise) ? 1 : 0;
			int sweep = clockwise ? 1 : 0;
			m_out << "A " << radius << " " << radius << " " << rotation << " " << largeArc << " "
			      << sweep << " " << to.x() << " " << -to.y() << " ";
			verticesToDraw.push_back(to);
			from = to;

		} else if (std::holds_alternative<RenderPath::Close>(c)) {
			m_out << "Z ";
		}
	}
	m_out << "\"/>\n";

	if (m_style.m_mode & vertices) {
		for (const Point<Inexact>& vertex : verticesToDraw) {
			draw(vertex);
		}
	}
}

void SvgRenderer::drawText(const Point<Inexact>& p, const std::string& text) {
	m_out << "<text text-anchor=\"middle\" dominant-baseline=\"middle\" x=\"" << p.x() << "\" y=\""
	      << -p.y() << "\">" << escapeForSvg(text) << "</text>\n";
}

void SvgRenderer::pushStyle() {
	m_styleStack.push(m_style);
}

void SvgRenderer::popStyle() {
	m_style = m_styleStack.top();
	m_styleStack.pop();
}

void SvgRenderer::setMode(int mode) {
	m_style.m_mode = mode;
}

void SvgRenderer::setStroke(Color color, double width, [[maybe_unused]] bool absoluteWidth) {
	m_style.m_strokeColor = "rgb(" + std::to_string(color.r) + ", " + std::to_string(color.g) +
	                        ", " + std::to_string(color.b) + ")";
	m_style.m_strokeWidth = width;
}

void SvgRenderer::setStrokeOpacity(int alpha) {
	m_style.m_strokeOpacity = alpha / 255.0;
}

void SvgRenderer::setFill(Color color) {
	m_style.m_fillColor = "rgb(" + std::to_string(color.r) + ", " + std::to_string(color.g) + ", " +
	                      std::to_string(color.b) + ")";
}

void SvgRenderer::setFillOpacity(int alpha) {
	m_style.m_fillOpacity = alpha / 255.0;
}

std::string SvgRenderer::convertPolygonToCurve(const Polygon<Inexact>& p) const {
	std::stringstream result;
	bool first = true;
	for (auto& vertex : p) {
		result << (first ? "M " : "L ") << vertex.x() << " " << -vertex.y() << " ";
		first = false;
	}
	result << "Z";
	return result.str();
}

std::string SvgRenderer::getStyle() const {
	if ((m_style.m_mode & GeometryRenderer::fill) && (m_style.m_mode & GeometryRenderer::stroke)) {
		return "fill=\"" + m_style.m_fillColor +
		       "\" fill-opacity=\"" + std::to_string(m_style.m_fillOpacity) +
		       "\" stroke=\"" + m_style.m_strokeColor +
		       "\" stroke-linecap=\"round" +
		       "\" stroke-linejoin=\"round" +
		       "\" stroke-opacity=\"" + std::to_string(m_style.m_strokeOpacity) +
		       "\" stroke-width=\"" + std::to_string(m_style.m_strokeWidth) + "\"";
	} else if (m_style.m_mode & GeometryRenderer::fill) {
		return "fill=\"" + m_style.m_fillColor +
		       "\" fill-opacity=\"" + std::to_string(m_style.m_fillOpacity) + "\"";
	} else {
		return "fill=\"none\" stroke=\"" + m_style.m_strokeColor +
		       "\" stroke-linecap=\"round" +
		       "\" stroke-linejoin=\"round" +
		       "\" stroke-opacity=\"" + std::to_string(m_style.m_strokeOpacity) +
		       "\" stroke-width=\"" + std::to_string(m_style.m_strokeWidth) + "\"";
	}
}

std::string SvgRenderer::getVertexStyle() const {
	if ((m_style.m_mode & GeometryRenderer::vertices)) {
		return "fill=\"" + m_style.m_strokeColor +
		       "\" fill-opacity=\"" + std::to_string(m_style.m_strokeOpacity) + "\"";
	} else {
		return "";
	}
}

void SvgRenderer::addPainting(const std::shared_ptr<GeometryPainting>& painting) {
	m_paintings.push_back(DrawnPainting{painting});
}

void SvgRenderer::addPainting(const std::shared_ptr<GeometryPainting>& painting, const std::string& name) {
	std::string spaceless;
	std::replace_copy(name.begin(), name.end(), std::back_inserter(spaceless), ' ', '_');
	m_paintings.push_back(DrawnPainting{painting, spaceless});
}

std::string SvgRenderer::escapeForSvg(const std::string& text) const {
	std::stringstream result;
	for (int i = 0; i < text.size(); i++) {
		switch (text[i]) {
		case '<':
			result << "&lt;";
			break;
		case '>':
			result << "&gt;";
			break;
		default:
			result << text[i];
		}
	}
	return result.str();
}

} // namespace cartocrow::renderer
