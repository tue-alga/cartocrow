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

#include "ipe_reader.h"

#include "cartocrow/core/cubic_bezier.h"

#include <CGAL/Boolean_set_operations_2/Gps_polygon_validation.h>

#include <ipebase.h>
#include <ipegeo.h>
#include <ipeshape.h>
#include <ipepath.h>

#include <fstream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

using namespace cartocrow::renderer;

namespace cartocrow {

std::shared_ptr<ipe::Document> IpeReader::loadIpeFile(const std::filesystem::path& filename) {
	std::fstream fin(filename);
	std::string input;
	if (fin) {
		using Iterator = std::istreambuf_iterator<char>;
		input.assign(Iterator(fin), Iterator());
	}

	ipe::Platform::initLib(ipe::IPELIB_VERSION);
	int load_reason = 0;
	ipe::Buffer buffer(input.c_str(), input.size());
	ipe::BufferSource bufferSource(buffer);
	ipe::FileFormat format = ipe::Document::fileFormat(bufferSource);
	ipe::Document* document = ipe::Document::load(bufferSource, format, load_reason);

	if (load_reason > 0) {
		throw std::runtime_error("Unable to load Ipe file: parse error at position " +
		                         std::to_string(load_reason));
	} else if (load_reason == ipe::Document::EVersionTooOld) {
		throw std::runtime_error("Unable to load Ipe file: the version of the file is too old");
	} else if (load_reason == ipe::Document::EVersionTooRecent) {
		throw std::runtime_error("Unable to load Ipe file: the file version is newer than Ipelib");
	} else if (load_reason == ipe::Document::EFileOpenError) {
		throw std::runtime_error("Unable to load Ipe file: error opening the file");
	} else if (load_reason == ipe::Document::ENotAnIpeFile) {
		throw std::runtime_error(
		    "Unable to load Ipe file: the file does not exist or was not created by Ipe");
	}

	return std::shared_ptr<ipe::Document>(document);
}

Color IpeReader::convertIpeColor(ipe::Color color) {
	return Color{static_cast<int>(color.iRed.toDouble() * 255),
	             static_cast<int>(color.iGreen.toDouble() * 255),
	             static_cast<int>(color.iBlue.toDouble() * 255)};
}

PolygonSet<Exact> IpeReader::convertShapeToPolygonSet(const ipe::Shape& shape,
                                                      const ipe::Matrix& matrix) {
	PolygonSet<Exact> set;
	for (int i = 0; i < shape.countSubPaths(); ++i) {
		Polygon<Exact> polygon;
		if (shape.subPath(i)->type() != ipe::SubPath::ECurve) {
			throw std::runtime_error("Encountered shape with a non-polygonal boundary");
		}
		const ipe::Curve* curve = shape.subPath(i)->asCurve();
		for (int j = 0; j < curve->countSegments(); ++j) {
			ipe::CurveSegment segment = curve->segment(j);
			if (segment.type() != ipe::CurveSegment::ESegment) {
				throw std::runtime_error("Encountered shape with a non-polygonal boundary");
			}
			if (j == 0) {
				ipe::Vector v = matrix * segment.cp(0);
				polygon.push_back(Point<Exact>(v.x, v.y));
			}
			ipe::Vector v = matrix * segment.last();
			Point<Exact> p(v.x, v.y);
			if (p != polygon.container().back()) {
				polygon.push_back(Point<Exact>(v.x, v.y));
			}
		}
		// if the begin and end vertices are equal, remove one of them
		if (polygon.container().front() == polygon.container().back()) {
			polygon.container().pop_back();
		}
		if (!polygon.is_simple()) {
//			std::cerr << "Encountered non-simple polygon" << std::endl;
//			continue;
			for (const auto v : polygon.vertices()) {
				std::cout << v << std::endl;
			}
			throw std::runtime_error("Encountered non-simple polygon");
		}
		if (polygon.is_clockwise_oriented()) {
			polygon.reverse_orientation();
		}
		set.symmetric_difference(PolygonWithHoles<Exact>(polygon));
	}
	return set;
}

CubicBezierSpline IpeReader::convertPathToSpline(const ipe::SubPath& path, const ipe::Matrix& matrix) {
	CubicBezierSpline spline;
	if (path.type() == ipe::SubPath::EClosedSpline) {
		std::vector<ipe::Bezier> beziers;
		path.asClosedSpline()->beziers(beziers);
		for (auto bezier : beziers) {
			spline.appendCurve(
			    Point<Inexact>(bezier.iV[0].x, bezier.iV[0].y), Point<Inexact>(bezier.iV[1].x, bezier.iV[1].y),
			    Point<Inexact>(bezier.iV[2].x, bezier.iV[2].y), Point<Inexact>(bezier.iV[3].x, bezier.iV[3].y));
		}
	} else {
		throw std::runtime_error("Only closed splines are supported for spline conversion");
	}
	return spline;
}

RenderPath IpeReader::convertShapeToRenderPath(const ipe::Shape& shape, const ipe::Matrix& matrix) {
    RenderPath renderPath;
    for (int i = 0; i < shape.countSubPaths(); ++i) {
        if (shape.subPath(i)->type() != ipe::SubPath::ECurve) {
            throw std::runtime_error("Encountered closed ellipse or B-spline; unimplemented");
        }
        const ipe::Curve* curve = shape.subPath(i)->asCurve();
        for (int j = 0; j < curve->countSegments(); ++j) {
            ipe::CurveSegment segment = curve->segment(j);
            Point<Inexact> last;
            if (segment.type() == ipe::CurveSegment::ESegment || segment.type() == ipe::CurveSegment::EArc) {
                if (j == 0) {
                    ipe::Vector v = matrix * segment.cp(0);
                    Point<Inexact> pt(v.x, v.y);
                    last = pt;
                    renderPath.moveTo(pt);
                }
                ipe::Vector v = matrix * segment.last();
                Point<Inexact> pt(v.x, v.y);
                if (pt != last) {
                    last = pt;
                    if (segment.type() == ipe::CurveSegment::ESegment) {
                        renderPath.lineTo(pt);
                    } else {
                        auto m = segment.matrix();
                        auto clockwise = m.a[3] < 0;
                        auto center = matrix * ipe::Vector(m.a[4], m.a[5]);
                        renderPath.arcTo({center.x, center.y}, clockwise, pt);
                    }
                }
            }
        }

        renderPath.close();
    }
    return renderPath;
}

RenderPath IpeReader::loadIpePath(const std::filesystem::path& ipeFile) {
    std::shared_ptr<ipe::Document> document = IpeReader::loadIpeFile(ipeFile);

    if (document->countPages() == 0) {
        throw std::runtime_error("Cannot read map from an Ipe file with no pages");
    } else if (document->countPages() > 1) {
        throw std::runtime_error("Cannot read map from an Ipe file with more than one page");
    }

    ipe::Page* page = document->page(0);

    for (int i = 0; i < page->count(); ++i) {
        ipe::Object* object = page->object(i);
        ipe::Object::Type type = object->type();
        if (type != ipe::Object::Type::EPath) {
            continue;
        }
        ipe::Path* path = object->asPath();
        ipe::Matrix matrix = path->matrix();
        ipe::Shape ipeShape = path->shape();
        auto renderPath = convertShapeToRenderPath(ipeShape, matrix);
        return renderPath;
    }

    throw std::runtime_error("Could not find a path in the ipe file");
}
} // namespace cartocrow
