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

#include <CGAL/enum.h>
#include <glog/logging.h>

#include <fstream>
#include <ipeshape.h>
#include <memory>
#include <optional>
#include <string>

namespace cartocrow::flow_map {

IpeReader::IpeReader() {}

bool IpeReader::readFile(const std::filesystem::path& filename, std::vector<Region>& regions,
                         std::vector<Region>& obstacles, std::vector<std::shared_ptr<Place>>& places) {
	std::shared_ptr<ipe::Document> document = cartocrow::IpeReader::loadIpeFile(filename);

	if (document->countPages() > 1) {
		LOG(INFO) << "Ipe file has more than one page; using the first page";
	}

	ipe::Page* page = document->page(0);

	for (int i = 0; i < page->count(); ++i) {
		ipe::Object* object = page->object(i);
		ipe::Object::Type type = object->type();
		int layer = page->layerOf(i);
		std::string layerName(page->layer(layer).data(), page->layer(layer).size());

		if (layerName == "regions" || layerName == "obstacles") {
			if (type != ipe::Object::Type::EPath) {
				LOG(INFO) << "Ignoring non-path element in layer " << layerName;
				continue;
			}
			ipe::Path* path = object->asPath();
			ipe::Matrix matrix = path->matrix();
			ipe::Shape shape = path->shape();
			std::vector<Polygon_with_holes> polygons =
			    cartocrow::IpeReader::convertShapeToPolygons(shape, matrix);
			Region region;
			region.shape.insert(region.shape.end(), polygons.begin(), polygons.end());

			if (layerName == "regions") {
				regions.push_back(region);
			} else {
				obstacles.push_back(region);
			}

		} else if (layerName == "places") {
			if (type != ipe::Object::Type::EText) {
				LOG(INFO) << "Ignoring non-text element in layer " << layerName;
				continue;
			}

			ipe::Matrix matrix = object->matrix();
			ipe::Vector translation = matrix * object->asText()->position();
			Point position(translation.x, translation.y);
			ipe::String ipeString = object->asText()->text();
			std::string text(ipeString.data(), ipeString.size());
			places.push_back(std::make_shared<Place>(text, PolarPoint{position}));

		} else {
			LOG(INFO) << "Ignoring element in layer \"" << layerName
			          << "\" (expected layers \"regions\", \"obstacles\", and \"places\")";
		}
	}

	return true;
}

} // namespace cartocrow::flow_map
