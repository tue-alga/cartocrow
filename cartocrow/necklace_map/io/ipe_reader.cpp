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

#include "cartocrow/core/io/ipe_reader.h"
#include "cartocrow/core/polygon.h"
#include "cartocrow/necklace_map/bezier_necklace.h"
#include "cartocrow/necklace_map/circle_necklace.h"
#include "cartocrow/necklace_map/necklace_shape.h"

#include <CGAL/enum.h>
#include <glog/logging.h>

#include <fstream>
#include <ipeshape.h>
#include <memory>
#include <optional>
#include <string>

namespace cartocrow::necklace_map {

IpeReader::IpeReader() {}

bool IpeReader::readFile(const std::filesystem::path& filename,
                         std::vector<necklace_map::MapElement::Ptr>& elements,
                         std::vector<necklace_map::Necklace::Ptr>& necklaces, Number& scale_factor) {
	std::shared_ptr<ipe::Document> document = cartocrow::IpeReader::loadIpeFile(filename);

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

	// step 2: find necklaces
	std::unordered_map<int, std::shared_ptr<Necklace>> necklaceForLayer;
	for (int i = 0; i < page->count(); ++i) {
		ipe::Object* object = page->object(i);
		int layer = page->layerOf(i);
		ipe::Object::Type type = object->type();
		if (type != ipe::Object::Type::EPath) {
			continue;
		}
		ipe::Path* path = object->asPath();
		ipe::Matrix matrix = path->matrix();
		ipe::Shape shape = path->shape();
		// interpret non-filled paths as necklaces
		if (path->pathMode() == ipe::TPathMode::EStrokedOnly) {
			int pathCount = shape.countSubPaths();
			if (pathCount > 1) {
				throw std::runtime_error("Found necklace with > 1 subpath");
			}
			const ipe::SubPath* p = shape.subPath(0);
			NecklaceShape::Ptr necklaceShape;
			if (p->type() == ipe::SubPath::EEllipse) {
				// circle necklace
				ipe::Matrix m = matrix * p->asEllipse()->matrix();
				ipe::Vector position = m.translation();
				double size_squared = m.a[0] * m.a[0];
				necklaceShape = std::make_shared<CircleNecklace>(
				    Circle(Point(position.x, position.y), size_squared));
			} else if (p->type() == ipe::SubPath::EClosedSpline) {
				Point kernel(300, 300); // TODO read kernel somehow from the Ipe file?
				necklaceShape = std::make_shared<BezierNecklace>(
				    cartocrow::IpeReader::convertPathToSpline(*p, matrix), kernel);
			}
			if (!necklaceShape) {
				throw std::runtime_error("Found necklace with invalid shape " +
				                         std::to_string(p->type()));
			}
			auto necklace = std::make_shared<Necklace>("necklace", necklaceShape);
			necklaces.push_back(necklace);
			necklaceForLayer[layer] = necklace;
		}
	}

	// step 3: find regions
	for (int i = 0; i < page->count(); ++i) {
		ipe::Object* object = page->object(i);
		int layer = page->layerOf(i);
		ipe::Object::Type type = object->type();
		if (type != ipe::Object::Type::EPath) {
			continue;
		}
		ipe::Path* path = object->asPath();
		ipe::Matrix matrix = path->matrix();
		ipe::Shape shape = path->shape();
		// interpret filled paths as regions
		if (path->pathMode() == ipe::TPathMode::EStrokedAndFilled) {
			std::vector<Polygon_with_holes> polygons =
			    cartocrow::IpeReader::convertShapeToPolygons(shape, matrix);
			std::optional<size_t> labelId = findLabelInside(polygons, labels);
			if (!labelId.has_value()) {
				LOG(WARNING) << "Ignoring region without label";
				continue;
			}
			labels[labelId.value()].matched = true;
			std::string name = labels[labelId.value()].text;
			auto element = std::make_shared<necklace_map::MapElement>(name);
			element->region.shape = polygons;
			element->color = cartocrow::IpeReader::convertIpeColor(path->fill().color());
			if (necklaceForLayer.find(layer) == necklaceForLayer.end()) {
				std::string layerName(page->layer(layer).data(), page->layer(layer).size());
				throw std::runtime_error("Encountered layer " + layerName + " without a necklace");
			}
			element->necklace = necklaceForLayer[layer];
			elements.push_back(element);
		}
	}

	return true;
}

std::optional<size_t> IpeReader::findLabelInside(std::vector<Polygon_with_holes>& polygons,
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
