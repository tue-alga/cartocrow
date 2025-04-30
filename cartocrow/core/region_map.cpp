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

#include <CGAL/enum.h>
#include <stdexcept>

#include "cartocrow/reader/ipe_reader.h"

#include "centroid.h"

namespace cartocrow {

RegionMap ipeToRegionMap(const std::filesystem::path& file, bool labelAtCentroid) {
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
        std::string name;
        if (labelAtCentroid) {
            auto& label = findLabelAtCentroid(shape, labels);
            name = label.text;
            if (label.matched) {
                std::cerr << "Label matched to multiple regions" << std::endl;
            }
            label.matched = true;
        } else {
            std::optional<size_t> labelId = findLabelInside(shape, labels);
            if (!labelId.has_value()) {
				std::vector<PolygonWithHoles<Exact>> pwhs;
				shape.polygons_with_holes(std::back_inserter(pwhs));
				for (const auto& pwh : pwhs) {
					std::cout << "Polygon: outer" << std::endl;
					for (const auto& v : pwh.outer_boundary().vertices()) {
						std::cout << v << " ";
					}
					std::cout << std::endl;

					for (const auto& h : pwh.holes()) {
						std::cout << "Polygon: hole" << std::endl;
						for (const auto& v : h.vertices()) {
							std::cout << v << " ";
						}
						std::cout << std::endl;
					}
				}
                throw std::runtime_error("Encountered region without a label");
            }
            labels[labelId.value()].matched = true;
            name = labels[labelId.value()].text;
        }
        if (regions.contains(name)) {
            Region& region = regions[name];
            region.shape.join(shape);
        } else {
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
	}

	return regions;
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

detail::RegionLabel& detail::findLabelAtCentroid(const PolygonSet<Exact>& shape,
                                                 std::vector<RegionLabel>& labels) {
    auto c = centroid(shape);
    RegionLabel& closest = labels.front();
    Number<Exact> minDist = CGAL::squared_distance(c, labels.front().position);
    for (auto lit = (++labels.begin()); lit != labels.end(); ++lit) {
        auto dist = CGAL::squared_distance(c, lit->position);
        if (dist < minDist) {
            closest = *lit;
            minDist = dist;
        }
    }
    return closest;
}
} // namespace cartocrow
