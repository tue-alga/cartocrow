/*
The Necklace Map library implements the algorithmic
geo-visualization method by the same name, developed by
Bettina Speckmann and Kevin Verbeek at TU Eindhoven
(DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
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
#include "cartocrow/necklace_map/circle_necklace.h"
#include "cartocrow/necklace_map/necklace_shape.h"

#include <CGAL/enum.h>
#include <glog/logging.h>

#include <fstream>
#include <memory>
#include <optional>
#include <string>

namespace cartocrow::necklace_map {

IpeReader::IpeReader() {}

bool IpeReader::readFile(const std::filesystem::path& filename,
                         std::vector<necklace_map::MapElement::Ptr>& elements,
                         std::vector<necklace_map::Necklace::Ptr>& necklaces, Number& scale_factor,
                         int max_retries) {
	std::string input;
	try {
		std::fstream fin(filename);
		if (fin) {
			using Iterator = std::istreambuf_iterator<char>;
			input.assign(Iterator(fin), Iterator());
		}
	} catch (const std::exception& e) {
		LOG(ERROR) << e.what();
	}

	ipe::Platform::initLib(70224);
	int load_reason = 0;
	ipe::Buffer buffer(input.c_str(), input.size());
	ipe::BufferSource bufferSource(buffer);
	ipe::FileFormat format = ipe::Document::fileFormat(bufferSource);
	ipe::Document* document = ipe::Document::load(bufferSource, format, load_reason);

	if (load_reason > 0) {
		LOG(ERROR) << "Unable to load Ipe file: parse error at position " << load_reason;
		return false;
	} else if (load_reason == ipe::Document::EVersionTooOld) {
		LOG(ERROR) << "Unable to load Ipe file: the version of the file is too old";
		return false;
	} else if (load_reason == ipe::Document::EVersionTooRecent) {
		LOG(ERROR) << "Unable to load Ipe file: the file version is newer than Ipelib";
		return false;
	} else if (load_reason == ipe::Document::EFileOpenError) {
		LOG(ERROR) << "Unable to load Ipe file: error opening the file";
		return false;
	} else if (load_reason == ipe::Document::ENotAnIpeFile) {
		LOG(ERROR) << "Unable to load Ipe file: the file was not created by Ipe";
		return false;
	}

	if (document->countPages() > 1) {
		LOG(INFO) << "Ipe file has more than one page; using the first page";
	}

	ipe::Page* page = document->page(0);

	// step 1: find labels
	std::vector<Label> labels;

	for (int i = 0; i < page->count(); ++i) {
		ipe::Object* object = page->object(i);
		ipe::Object::Type type = object->type();
		if (type != ipe::Object::Type::EText) {
			continue;
		}
		ipe::Matrix matrix = object->matrix();
		ipe::Vector translation = matrix * object->asText()->position();
		Point position(translation.x, translation.y);
		ipe::String ipeString = object->asText()->text();
		std::string text(ipeString.data(), ipeString.size());
		labels.push_back(Label{position, text, false});
	}

	for (Label label : labels) {
		std::cout << "[" << label.text << "] " << label.position.x() << " " << label.position.y()
		          << std::endl;
	}

	// step 2: find regions
	for (int i = 0; i < page->count(); ++i) {
		ipe::Object* object = page->object(i);
		ipe::Object::Type type = object->type();
		if (type != ipe::Object::Type::EPath) {
			continue;
		}
		ipe::Path* path = object->asPath();
		ipe::Matrix matrix = path->matrix();
		ipe::Shape shape = path->shape();
		// interpret stroked paths as necklaces; others are considered regions
		if (path->pathMode() == ipe::TPathMode::EStrokedOnly) {
			int pathCount = shape.countSubPaths();
			if (pathCount > 1) {
				LOG(ERROR) << "Unable to load Ipe file: found necklace with > 1 subpath";
				return false;
			}
			const ipe::SubPath* p = shape.subPath(0);
			NecklaceShape::Ptr necklaceShape;
			if (p->type() == ipe::SubPath::EEllipse) {
				ipe::Matrix m = matrix * p->asEllipse()->matrix();
				ipe::Vector position = m.translation();
				double size_squared = m.a[0] * m.a[0];
				necklaceShape = std::make_shared<CircleNecklace>(
				    Circle(Point(position.x, position.y), size_squared));
				std::cout << position.x << " " << position.y << " " << size_squared << " bla"
				          << std::endl;
			}
			if (!necklaceShape) {
				LOG(ERROR) << "Unable to load Ipe file: found necklace with invalid shape";
				return false;
			}
			auto necklace = std::make_shared<Necklace>("necklace", necklaceShape);
			necklaces.push_back(necklace);
		} else {
			Region::PolygonSet polygons = convertIpeShape(shape, matrix);
			std::optional<size_t> labelId = findLabelInside(polygons, labels);
			if (!labelId.has_value()) {
				LOG(WARNING) << "Ignoring region without label";
				continue;
			}
			labels[labelId.value()].matched = true;
			std::string name = labels[labelId.value()].text;
			auto element = std::make_shared<necklace_map::MapElement>(name);
			element->region.shape = polygons;
			element->color = convertIpeColor(path->fill().color());
			elements.push_back(element);
		}
	}

	// step 3: assign labels to polygons

	// TODO: temporarily assign everything to the first necklace
	for (auto element : elements) {
		element->necklace = necklaces[0];
	}

	std::cout << "Done reading!" << std::endl;

	return true;
}

Region::PolygonSet IpeReader::convertIpeShape(ipe::Shape shape, ipe::Matrix matrix) {
	Region::PolygonSet polygons;
	for (int i = 0; i < shape.countSubPaths(); i++) {
		Polygon polygon;
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

Color IpeReader::convertIpeColor(ipe::Color color) {
	return Color{static_cast<int>(color.iRed.toDouble() * 255),
	             static_cast<int>(color.iGreen.toDouble() * 255),
	             static_cast<int>(color.iBlue.toDouble() * 255)};
}

std::optional<size_t> IpeReader::findLabelInside(Region::PolygonSet& polygons,
                                                 std::vector<Label>& labels) {
	for (size_t i = 0; i < labels.size(); ++i) {
		Label& label = labels[i];
		for (Polygon_with_holes p : polygons) {
			if (!label.matched &&
			    p.outer_boundary().bounded_side(label.position) == CGAL::ON_BOUNDED_SIDE) {
				return std::make_optional(i);
			}
		}
	}
	return std::nullopt;
}

} // namespace cartocrow::necklace_map
