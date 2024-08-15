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

#include "region_map.h"

#include <ipedoc.h>
#include <ipepath.h>
#include <ipereference.h>

#include <CGAL/enum.h>
#include <stdexcept>

#include "ipe_reader.h"
namespace cartocrow {

RegionMap ipeToRegionMap(const std::filesystem::path& file) {
	RegionMap regions;

	std::shared_ptr<ipe::Document> document = IpeReader::loadIpeFile(file);

	if (document->countPages() == 0) {
		throw std::runtime_error("Cannot read map from an Ipe file with no pages");
	} else if (document->countPages() > 1) {
		throw std::runtime_error("Cannot read map from an Ipe file with more than one page");
	}

	ipe::Page* page = document->page(0);

	// step 1: find labels
	std::vector<detail::RegionLabel> labels;

	for (int i = 0; i < page->count(); ++i) {
		ipe::Object* object = page->object(i);
		ipe::Object::Type type = object->type();
		if (type != ipe::Object::Type::EText) {
			continue;
		}
		ipe::Matrix matrix = object->matrix();
		ipe::Vector translation = matrix * object->asText()->position();
		Point<Exact> position(translation.x, translation.y);
		ipe::String ipeString = object->asText()->text();
		std::string text(ipeString.data(), ipeString.size());
		labels.push_back(detail::RegionLabel{position, text, false});
	}

	// step 2: find regions
	for (int i = 0; i < page->count(); ++i) {
		ipe::Object* object = page->object(i);
		int layer = page->layerOf(i);
		ipe::Object::Type type = object->type();
		if (type != ipe::Object::Type::EPath) {
			continue;
		}
		ipe::Path* path = object->asPath();
		ipe::Matrix matrix = path->matrix();
		ipe::Shape ipeShape = path->shape();
		// interpret filled paths as regions
		PolygonSet<Exact> shape = cartocrow::IpeReader::convertShapeToPolygonSet(ipeShape, matrix);
		std::optional<size_t> labelId = findLabelInside(shape, labels);
		if (!labelId.has_value()) {
			throw std::runtime_error("Encountered region without a label");
		}
		labels[labelId.value()].matched = true;
		std::string name = labels[labelId.value()].text;
		Region region;
		region.name = name;
		if (path->fill().isSymbolic()) {
			region.color = IpeReader::convertIpeColor(
			    document->cascade()->find(ipe::Kind::EColor, path->fill()).color());
		} else {
			region.color = IpeReader::convertIpeColor(path->fill().color());
		}
		region.shape = shape;
		regions[name] = region;
	}

	return regions;
}

std::vector<Point<Exact>> ipeToSalientPoints(const std::filesystem::path& file) {
	std::shared_ptr<ipe::Document> document = IpeReader::loadIpeFile(file);
	ipe::Page* page = document->page(0);

	std::vector<Point<Exact>> points;
	for (int i = 0; i < page->count(); ++i) {
		ipe::Object* object = page->object(i);
		ipe::Object::Type type = object->type();
		if (type != ipe::Object::Type::EReference) {
			continue;
		}
		ipe::Reference* symbol = object->asReference();
		ipe::Vector position = symbol->position();
		points.push_back(Point<Exact>(position.x, position.y));
	}

	return points;
}

std::optional<size_t> detail::findLabelInside(const PolygonSet<Exact>& shape,
                                              const std::vector<RegionLabel>& labels) {
	std::optional<size_t> labelId;
	for (size_t i = 0; i < labels.size(); ++i) {
		const RegionLabel& label = labels[i];
		if (!label.matched && shape.oriented_side(label.position) == CGAL::ON_POSITIVE_SIDE) {
			if (labelId.has_value()) {
				throw std::runtime_error("Encountered region with more than one label");
			}
			labelId = std::make_optional(i);
		}
	}
	return labelId;
}

} // namespace cartocrow
