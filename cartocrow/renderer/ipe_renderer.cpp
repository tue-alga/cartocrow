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

#include "function_painting.h"
#include "geometry_renderer.h"

#include <ipeattributes.h>
#include <ipebase.h>
#include <ipedoc.h>
#include <ipegeo.h>
#include <ipeiml.h>
#include <ipereference.h>
#include <ipestyle.h>
#include <ipetext.h>

#include <string>

namespace cartocrow::renderer {

IpeRenderer::IpeRenderer(const std::shared_ptr<GeometryPainting>& painting) {
	m_paintings.push_back(DrawnPainting{painting});
}

IpeRenderer::IpeRenderer(const std::shared_ptr<GeometryPainting>& painting, const std::string& name) {
	m_paintings.push_back(DrawnPainting{painting, name});
}

void IpeRenderer::save(const std::filesystem::path& file) {
	ipe::Platform::initLib(ipe::IPELIB_VERSION);
	ipe::Document document;
	ipe::Layout layout;
	layout.iOrigin = ipe::Vector(0, 0);
	layout.iPaperSize = ipe::Vector(1000, 1000);
	layout.iFrameSize = ipe::Vector(1000, 1000);
	layout.iCrop = true;

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
	setStrokeOpacity(255); // add default alpha to style sheet

	auto latexPreambleSheet = new ipe::StyleSheet();
	latexPreambleSheet->setPreamble(m_preamble.c_str());
	latexPreambleSheet->setName("latex-preamble");
	document.cascade()->insert(3, latexPreambleSheet);

	m_page = new ipe::Page();
	document.push_back(m_page);

	int current_page = 0;

	for (const auto& painting : m_paintings) { // Assumes m_paintings are ordered in increasing page_index
		while (painting.page_index > current_page) {
			m_page = new ipe::Page();
			document.push_back(m_page);
			++current_page;
		}
		pushStyle();
		if (auto name = painting.name) {
			m_page->addLayer(name->c_str());
		} else {
			m_page->addLayer();
		}
		m_layer = m_page->countLayers() - 1;
		painting.m_painting->paint(*this);
		popStyle();
	}

	auto pdf = file.extension() == ".pdf";
	if (pdf) {
		bool success = document.runLatex(file.c_str());
		if (!success) {
			std::cerr << "LaTeX compilation failed." << std::endl;
		}
	}
	// todo: fix saving to pdf. Currently on opening it says the pdf has no pages; but opening via ipe works.
	auto success = document.save(file.string().c_str(), pdf ? ipe::FileFormat::Pdf : ipe::FileFormat::Xml, 0);
	if (!success) {
		std::cerr << "Saving failed." << std::endl;
	}
}

void IpeRenderer::draw(const Point<Inexact>& p) {
	ipe::Vector vector(p.x(), p.y());
	ipe::Attribute name = ipe::Attribute(true, "mark/disk(sx)");
	ipe::Reference* reference = new ipe::Reference(getAttributesForStyle(), name, vector);
	m_page->append(ipe::TSelect::ENotSelected, m_layer, reference);
}

void IpeRenderer::draw(const Line<Inexact>& l) {
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

void IpeRenderer::draw(const Ray<Inexact>& r) {
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

void IpeRenderer::draw(const Halfplane<Inexact>& h) {
	// crop to document size
	auto l = h.line();
	auto bounds = CGAL::Iso_rectangle_2<Inexact>(CGAL::ORIGIN, Point<Inexact>(1000.0, 1000.0));
	auto result = intersection(l, bounds);
	if (result) {
		if (const Segment<Inexact>* s = boost::get<Segment<Inexact>>(&*result)) {
			int oldMode = m_style.m_mode;
			if (oldMode & fill) {
				// Draw filled half-plane
				setMode(fill);
				Rectangle<Inexact> rect(bounds.xmin(), bounds.ymin(), bounds.xmax(), bounds.ymax());
				Polygon<Inexact> poly = h.polygon(rect);
				GeometryRenderer::draw(poly);
			}
			setMode(oldMode & ~vertices);
			GeometryRenderer::draw(*s);
			setMode(oldMode);
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
	drawPathOnPage(path);
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
	drawPathOnPage(path);

	if (m_style.m_mode & vertices) {
		for (BezierCurve c : s.curves()) {
			draw(c.source());
		}
		draw(s.curves().back().target());
	}
}

ipe::Shape* renderPathToIpe(const RenderPath& p) {
    auto* shape = new ipe::Shape();
    ipe::Curve* curve = nullptr;
    Point<Inexact> from;
    for (RenderPath::Command c : p.commands()) {
        if (std::holds_alternative<RenderPath::MoveTo>(c)) {
            if (curve) {
                shape->appendSubPath(curve);
            }
            curve = new ipe::Curve();
            Point<Inexact> to = std::get<RenderPath::MoveTo>(c).m_to;
            from = to;
            continue;
        }
        if (!curve) {
            // didn't start with MoveTo
            curve = new ipe::Curve();
        }
        if (std::holds_alternative<RenderPath::LineTo>(c)) {
            Point<Inexact> to = std::get<RenderPath::LineTo>(c).m_to;
            curve->appendSegment(ipe::Vector(from.x(), from.y()), ipe::Vector(to.x(), to.y()));
            from = to;

        } else if (std::holds_alternative<RenderPath::ArcTo>(c)) {
            Point<Inexact> center = std::get<RenderPath::ArcTo>(c).m_center;
            Point<Inexact> to = std::get<RenderPath::ArcTo>(c).m_to;
            bool clockwise = std::get<RenderPath::ArcTo>(c).m_clockwise;

            double radius = sqrt((center - to).squared_length());
            ipe::Matrix matrix(radius, 0, 0, clockwise ? -radius : radius, center.x(), center.y());
            curve->appendArc(matrix, ipe::Vector(from.x(), from.y()), ipe::Vector(to.x(), to.y()));
            from = to;

        } else if (std::holds_alternative<RenderPath::Close>(c)) {
            curve->setClosed(true);
        }
    }
    if (curve) {
        shape->appendSubPath(curve);
    }
    return shape;
}

void IpeRenderer::draw(const RenderPath& p) {
	std::vector<Point<Inexact>> verticesToDraw;
    p.vertices(std::back_inserter(verticesToDraw));
	if (p.commands().size() > 1) {
		ipe::Shape* shape = renderPathToIpe(p);
		auto* path = new ipe::Path(getAttributesForStyle(), *shape);
		drawPathOnPage(path);
	}

	if (m_style.m_mode & vertices) {
		for (const Point<Inexact>& vertex : verticesToDraw) {
			draw(vertex);
		}
	}
}

void IpeRenderer::drawText(const Point<Inexact>& p, const std::string& text, bool escape) {
	ipe::String labelText = escape ? escapeForLaTeX(text).data() : text.data();
	ipe::Text* label = new ipe::Text(getAttributesForStyle(), labelText,
	                                 ipe::Vector(p.x(), p.y()), ipe::Text::TextType::ELabel);
    label->setHorizontalAlignment(m_style.m_horizontalTextAlignment);
    label->setVerticalAlignment(m_style.m_verticalTextAlignment);
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

void IpeRenderer::setStroke(Color color, double width, [[maybe_unused]] bool absoluteWidth) {
	const double factor = 1000.0 / 255.0;
	m_style.m_strokeColor = ipe::Color(color.r * factor, color.g * factor, color.b * factor);
	m_style.m_strokeWidth = width;
}

ipe::Attribute IpeRenderer::opacity_attribute(int alpha) {
	// Ipe does not allow arbitrary opacity values; it only allows symbolic
	// references to alpha values from the stylesheet.
	// Therefore, we check if the requested opacity value already exists. If
	// not, we add it to the stylesheet.
	ipe::Attribute name = ipe::Attribute(true, std::to_string(alpha).data());
	if (!m_alphaSheet->has(ipe::Kind::EOpacity, name)) {
		m_alphaSheet->add(ipe::Kind::EOpacity, name,
		                  ipe::Attribute(ipe::Fixed::fromDouble(alpha / 255.0)));
	}
	return name;
}

void IpeRenderer::setStrokeOpacity(int alpha) {
	auto name = opacity_attribute(alpha);
	m_style.m_strokeOpacity = name;
}

void IpeRenderer::setFill(Color color) {
	const double factor = 1000.0 / 255.0;
	m_style.m_fillColor = ipe::Color(color.r * factor, color.g * factor, color.b * factor);
}

void IpeRenderer::setFillOpacity(int alpha) {
	auto name = opacity_attribute(alpha);
	m_style.m_fillOpacity = name;
}

void IpeRenderer::setClipPath(const RenderPath &clipPath) {
    m_style.m_clipPath = renderPathToIpe(clipPath);
}

void IpeRenderer::setClipping(bool enable) {
    m_style.m_clip = enable;
}

void IpeRenderer::setLineCap(LineCap lineCap) {
	switch(lineCap) {
	case ButtCap: {
		m_style.m_lineCap = ipe::EButtCap;
		break;
	}
	case RoundCap: {
		m_style.m_lineCap = ipe::ERoundCap;
		break;
	}
	case SquareCap: {
		m_style.m_lineCap = ipe::ESquareCap;
		break;
	}
	}
}

void IpeRenderer::setLineJoin(LineJoin lineJoin) {
	switch(lineJoin) {
	case BevelJoin: {
		m_style.m_lineJoin = ipe::EBevelJoin;
		break;
	}
	case MiterJoin: {
		m_style.m_lineJoin = ipe::EMiterJoin;
		break;
	}
	case RoundJoin: {
		m_style.m_lineJoin = ipe::ERoundJoin;
		break;
	}
	}
}

void IpeRenderer::setHorizontalTextAlignment(HorizontalTextAlignment alignment) {
	switch(alignment) {
	case AlignHCenter: {
		m_style.m_horizontalTextAlignment = ipe::EAlignHCenter;
		break;
	}
	case AlignLeft: {
		m_style.m_horizontalTextAlignment = ipe::EAlignLeft;
		break;
	}
	case AlignRight: {
		m_style.m_horizontalTextAlignment = ipe::EAlignRight;
		break;
	}
	}
}

void IpeRenderer::setVerticalTextAlignment(VerticalTextAlignment alignment) {
	switch(alignment) {
	case AlignVCenter: {
		m_style.m_verticalTextAlignment = ipe::EAlignVCenter;
		break;
	}
	case AlignTop: {
		m_style.m_verticalTextAlignment = ipe::EAlignTop;
		break;
	}
	case AlignBottom: {
		m_style.m_verticalTextAlignment = ipe::EAlignBottom;
		break;
	}
	case AlignBaseline: {
		m_style.m_verticalTextAlignment = ipe::EAlignBaseline;
		break;
	}
	}
}

void IpeRenderer::drawPathOnPage(ipe::Path* path) {
	path->setLineCap(m_style.m_lineCap);
	path->setLineJoin(m_style.m_lineJoin);
	if (m_style.m_clip) {
		auto* group = new ipe::Group();
		group->push_back(path);
		group->setClip(*m_style.m_clipPath);
		m_page->append(ipe::TSelect::ENotSelected, m_layer, group);
	} else {
		m_page->append(ipe::TSelect::ENotSelected, m_layer, path);
	}
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
	attributes.iStrokeOpacity = m_style.m_strokeOpacity;
	return attributes;
}


void IpeRenderer::setPreamble(const std::string& preamble) {
	m_preamble = preamble;
}

void IpeRenderer::addPainting(const std::function<void(renderer::GeometryRenderer&)>& draw_function) {
	auto painting = std::make_shared<FunctionPainting>(draw_function);
	addPainting(painting);
}

void IpeRenderer::addPainting(const std::function<void(renderer::GeometryRenderer&)>& draw_function, const std::string& name) {
	auto painting = std::make_shared<FunctionPainting>(draw_function);
	addPainting(painting, name);
}

void IpeRenderer::addPainting(const std::shared_ptr<GeometryPainting>& painting) {
	m_paintings.push_back(DrawnPainting{painting, std::nullopt, m_pageIndex});
}

void IpeRenderer::addPainting(const std::shared_ptr<GeometryPainting>& painting, const std::string& name) {
	std::string spaceless;
	std::replace_copy(name.begin(), name.end(), std::back_inserter(spaceless), ' ', '_');
	m_paintings.push_back(DrawnPainting{painting, spaceless, m_pageIndex});
}

void IpeRenderer::nextPage() {
	++m_pageIndex;
}

int IpeRenderer::currentPage() {
	return m_pageIndex;
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
