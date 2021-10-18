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
#include "cartocrow/common/polygon.h"

#include <ipebase.h>
#include <ipegeo.h>
#include <ipeshape.h>

#include <fstream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

namespace cartocrow {

std::shared_ptr<ipe::Document> IpeReader::loadIpeFile(const std::filesystem::path& filename) {
	std::fstream fin(filename);
	std::string input;
	if (fin) {
		using Iterator = std::istreambuf_iterator<char>;
		input.assign(Iterator(fin), Iterator());
	}

	ipe::Platform::initLib(70224);
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
		throw std::runtime_error("Unable to load Ipe file: the file was not created by Ipe");
	}

	return std::shared_ptr<ipe::Document>(document);
}

Color IpeReader::convertIpeColor(ipe::Color color) {
	return Color{static_cast<int>(color.iRed.toDouble() * 255),
	             static_cast<int>(color.iGreen.toDouble() * 255),
	             static_cast<int>(color.iBlue.toDouble() * 255)};
}

std::vector<Polygon_with_holes> IpeReader::convertShapeToPolygons(const ipe::Shape& shape,
                                                                  const ipe::Matrix& matrix) {
	std::vector<Polygon_with_holes> polygons;
	for (int i = 0; i < shape.countSubPaths(); ++i) {
		Polygon polygon;
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
				polygon.push_back(Point(v.x, v.y));
			}
			ipe::Vector v = matrix * segment.last();
			polygon.push_back(Point(v.x, v.y));
		}
		polygons.push_back(Polygon_with_holes(polygon));
	}
	return polygons;
}

BezierSpline IpeReader::convertPathToSpline(const ipe::SubPath& path, const ipe::Matrix& matrix) {
	BezierSpline spline;
	if (path.type() == ipe::SubPath::EClosedSpline) {
		std::vector<ipe::Bezier> beziers;
		path.asClosedSpline()->beziers(beziers);
		for (auto bezier : beziers) {
			spline.AppendCurve(
			    Point(bezier.iV[0].x, bezier.iV[0].y), Point(bezier.iV[1].x, bezier.iV[1].y),
			    Point(bezier.iV[2].x, bezier.iV[2].y), Point(bezier.iV[3].x, bezier.iV[3].y));
		}
	} else {
		throw std::runtime_error("Only closed splines are supported for spline conversion");
	}
	return spline;
}

} // namespace cartocrow
