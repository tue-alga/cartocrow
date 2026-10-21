/*
Copyright (C) 2026  TU Eindhoven

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
#include "function_painting.h"

#include <CGAL/enum.h>

#include <cmath>
#include <string>

namespace cartocrow::renderer {

SvgRenderer::SvgRenderer(const std::shared_ptr<GeometryPainting>& painting) {
	m_paintings.push_back(DrawnPainting{painting});
}

SvgRenderer::SvgRenderer(const std::shared_ptr<GeometryPainting>& painting, const std::string& name) {
	m_paintings.push_back(DrawnPainting{painting, name});
}

void SvgRenderer::savePaintings() {
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
}

void SvgRenderer::save(const std::filesystem::path& file) {
	std::locale::global(std::locale("C"));
	m_out.open(file);
	m_out << "<svg version=\"1.1\" xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" xmlns=\"http://www.w3.org/2000/svg\">\n";
	savePaintings();
	m_out << "</svg>\n";
	m_out.close();
}

void SvgRenderer::save(const std::filesystem::path& file, Box viewBox) {
	std::locale::global(std::locale("C"));
	m_out.open(file);
	m_out << "<svg version=\"1.1\"" << " viewBox=\"" <<
	    viewBox.xmin() << " " << -viewBox.ymax() << " " << viewBox.x_span() << " " << viewBox.y_span() <<
	    "\" xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" xmlns=\"http://www.w3.org/2000/svg\">\n";
	savePaintings();
	m_out << "</svg>\n";
	m_out.close();
}

void SvgRenderer::draw(const Point<Inexact>& p) {
	m_out << "<circle " << getVertexStyle() << " r=\"" << m_style.m_pointSize / 2 << "\" cx=\"" << p.x()
	      << "\" cy=\"" << -p.y() << "\"/>\n";
}

void SvgRenderer::draw(const Line<Inexact>& l) {
	// crop to document size
	auto bounds = CGAL::Iso_rectangle_2<Inexact>(CGAL::ORIGIN, Point<Inexact>(1000.0, 1000.0));
	auto result = intersection(l, bounds);
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

void SvgRenderer::draw(const Ray<Inexact>& r) {
	// crop to document size
	auto bounds = CGAL::Iso_rectangle_2<Inexact>(CGAL::ORIGIN, Point<Inexact>(1000.0, 1000.0));
	auto result = intersection(r, bounds);
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

void SvgRenderer::draw(const Halfplane<Inexact>& h) {
	// crop to document size
	auto l = h.line();
	auto bounds = CGAL::Iso_rectangle_2<Inexact>(CGAL::ORIGIN, Point<Inexact>(1000.0, 1000.0));
	auto result = intersection(l, bounds);
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

void SvgRenderer::draw(const Circle<Inexact>& c) {
	double r = sqrt(c.squared_radius());
	m_out << "<circle " << getStyle() << " r=\"" << r << "\" cx=\"" << c.center().x() << "\" cy=\""
	      << -c.center().y() << "\"/>\n";
}

void SvgRenderer::draw(const Ellipse& e) {
	// TODO
	std::cerr << "Ellipsoid rendering is not implemented for svg format; ignoring\n";
}

void SvgRenderer::draw(const CubicBezierSpline& s) {
	// TODO
	std::cerr << "The SVG renderer does not support CubicBezierSplines; ignoring\n";
}

std::string renderPathToSVGCommands(const RenderPath& p, bool invertY) {
    std::stringstream ss;

    Point<Inexact> from;
    for (RenderPath::Command c : p.commands()) {
        if (std::holds_alternative<RenderPath::MoveTo>(c)) {
            Point<Inexact> to = std::get<RenderPath::MoveTo>(c).m_to;
            ss << "M " << to.x() << " " << (invertY ? -to.y() : to.y()) << " ";
            from = to;

        } else if (std::holds_alternative<RenderPath::LineTo>(c)) {
            Point<Inexact> to = std::get<RenderPath::LineTo>(c).m_to;
            ss << "L " << to.x() << " " << (invertY ? -to.y() : to.y()) << " ";
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
			if (!invertY) { sweep = !sweep; }
            ss << "A " << radius << " " << radius << " " << rotation << " " << largeArc << " "
                  << sweep << " " << to.x() << " " << (invertY ? -to.y() : to.y()) << " ";
            from = to;

        } else if (std::holds_alternative<RenderPath::Close>(c)) {
            ss << "Z ";
        }
    }

    return ss.str();
}

void SvgRenderer::draw(const RenderPath& p) {
    m_out << "<path " << getStyle() << " d=\"";
    m_out << renderPathToSVGCommands(p);
    m_out << "\"/>\n";

    std::vector<Point<Inexact>> verticesToDraw;
    p.vertices(std::back_inserter(verticesToDraw));
	if (m_style.m_mode & vertices) {
		for (const Point<Inexact>& vertex : verticesToDraw) {
			draw(vertex);
		}
	}
}

void SvgRenderer::drawText(const Point<Inexact>& p, const std::string& text, bool escape) {
	m_out << "<text text-anchor=\"" << m_style.m_horizontalTextAlignment
          << "\" dominant-baseline=\"" << m_style.m_verticalTextAlignment
		  << (m_style.m_fontFamily.has_value() ? ("\" font-family=\"" + *m_style.m_fontFamily) : "")
		  << (m_style.m_fontSize.has_value() ? ("\" font-size=\"" + std::to_string(*m_style.m_fontSize)) : "")
		  << (m_style.m_bold ? ("\" font-weight=\"bold") : "")
		  << "\" fill=\"" << m_style.m_strokeColor // ipe and qt use stroke color for text color so we do the same here
          << "\" x=\"" << p.x() << "\" y=\""
	      << -p.y() << "\">" << (escape ? escapeForSvg(text) : text) << "</text>\n";
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

void SvgRenderer::setPointSize(double size, [[maybe_unused]] bool absoluteSize) {
	m_style.m_pointSize = size;
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
    std::string clip = (!m_style.m_clipping || !m_style.m_clipPath.has_value()) ? "" : ("clip-path=\"url(#clipPath_" + std::to_string(*(m_style.m_clipPath)) + ")\" ");
	if ((m_style.m_mode & GeometryRenderer::fill) && (m_style.m_mode & GeometryRenderer::stroke)) {
		return clip +
               "fill=\"" + m_style.m_fillColor +
		       "\" fill-opacity=\"" + std::to_string(m_style.m_fillOpacity) +
		       "\" stroke=\"" + m_style.m_strokeColor +
		       "\" stroke-linecap=\"" + m_style.m_lineCap +
		       "\" stroke-linejoin=\"" + m_style.m_lineJoin +
		       "\" stroke-opacity=\"" + std::to_string(m_style.m_strokeOpacity) +
		       "\" stroke-width=\"" + std::to_string(m_style.m_strokeWidth) + "\"";
	} else if (m_style.m_mode & GeometryRenderer::fill) {
		return clip +
               "fill=\"" + m_style.m_fillColor +
		       "\" fill-opacity=\"" + std::to_string(m_style.m_fillOpacity) + "\"";
	} else {
		return clip +
               "fill=\"none\" stroke=\"" + m_style.m_strokeColor +
                "\" stroke-linecap=\"" + m_style.m_lineCap +
                "\" stroke-linejoin=\"" + m_style.m_lineJoin +
		       "\" stroke-opacity=\"" + std::to_string(m_style.m_strokeOpacity) +
		       "\" stroke-width=\"" + std::to_string(m_style.m_strokeWidth) + "\"";
	}
}

std::string SvgRenderer::getVertexStyle() const {
	return "fill=\"" + m_style.m_strokeColor +
	       "\" fill-opacity=\"" + std::to_string(m_style.m_strokeOpacity) + "\"";
}

void SvgRenderer::addPainting(const std::shared_ptr<GeometryPainting>& painting) {
	m_paintings.push_back(DrawnPainting{painting});
}

void SvgRenderer::addPainting(const std::shared_ptr<GeometryPainting>& painting, const std::string& name) {
	std::string spaceless;
	std::replace_copy(name.begin(), name.end(), std::back_inserter(spaceless), ' ', '_');
	m_paintings.push_back(DrawnPainting{painting, spaceless});
}

void SvgRenderer::addPainting(const std::function<void(renderer::GeometryRenderer&)>& draw_function) {
	auto painting = std::make_shared<FunctionPainting>(draw_function);
	addPainting(painting);
}

void SvgRenderer::addPainting(const std::function<void(renderer::GeometryRenderer&)>& draw_function, const std::string& name) {
	auto painting = std::make_shared<FunctionPainting>(draw_function);
	addPainting(painting, name);
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

void SvgRenderer::setClipPath(const RenderPath& clipPath) {
    m_out << "<clipPath id=\"clipPath_" << m_clipPathId << "\">";
    m_out << "<path " << " d=\"";
    m_out << renderPathToSVGCommands(clipPath);
    m_out << "\"/>\n";
    m_out << "</clipPath>";
    m_style.m_clipPath = m_clipPathId;
    ++m_clipPathId;
}

void SvgRenderer::setClipping(bool enable) {
    m_style.m_clipping = enable;
}

void SvgRenderer::setLineJoin(LineJoin lineJoin) {
    switch (lineJoin) {
        case RoundJoin: {
            m_style.m_lineJoin = "round";
            break;
        }
        case BevelJoin: {
            m_style.m_lineJoin = "bevel";
            break;
        }
        case MiterJoin: {
            m_style.m_lineJoin = "miter";
            break;
        }
    }
}

void SvgRenderer::setLineCap(LineCap lineCap) {
    switch (lineCap) {
        case RoundCap: {
            m_style.m_lineCap = "round";
            break;
        }
        case ButtCap: {
            m_style.m_lineCap = "butt";
            break;
        }
        case SquareCap: {
            m_style.m_lineCap = "square";
            break;
        }
    }
}

void SvgRenderer::setHorizontalTextAlignment(HorizontalTextAlignment alignment) {
    switch (alignment) {
        case AlignHCenter: {
            m_style.m_horizontalTextAlignment = "middle";
            break;
        }
        case AlignLeft: {
            m_style.m_horizontalTextAlignment = "start";
            break;
        }
        case AlignRight: {
            m_style.m_horizontalTextAlignment = "end";
            break;
        }
    }
}

void SvgRenderer::setVerticalTextAlignment(VerticalTextAlignment alignment) {
    switch (alignment) {
        case AlignVCenter: {
            m_style.m_verticalTextAlignment = "middle";
            break;
        }
        case AlignTop: {
            m_style.m_verticalTextAlignment = "hanging";
            break;
        }
        case AlignBottom: {
            m_style.m_verticalTextAlignment = "ideographic";
            break;
        }
        case AlignBaseline: {
            m_style.m_verticalTextAlignment = "alphabetic";
            break;
        }
    }
}

void SvgRenderer::setFontFamily(std::string fontFamily) {
	m_style.m_fontFamily = fontFamily;
}

void SvgRenderer::setFontSize(double fontSize) {
	m_style.m_fontSize = fontSize;
}

void SvgRenderer::setFontWeight(bool bold) {
	m_style.m_bold = bold;
}

void SvgRenderer::useDefaultFont() { 
	m_style.m_fontSize = std::nullopt;
	m_style.m_fontFamily = std::nullopt;
}
} // namespace cartocrow::renderer
