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

#include "region_map_reader.h"

#include <ipedoc.h>
#include <ipepath.h>
#include <ipereference.h>

#include <CGAL/enum.h>
#include <stdexcept>

#include "cartocrow/reader/ipe_reader.h"

#include "cartocrow/core/polygon_helpers.h"

namespace cartocrow {

RegionMap ipeToRegionMap(const std::filesystem::path& file, bool labelAtCentroid) {
	RegionMap regions;

	IpeReader reader(file);

	int numPages = reader.numberOfPages();
	if (numPages == 0) {
		throw std::runtime_error("Cannot read map from an Ipe file with no pages");
	} else if (numPages > 1) {
		throw std::runtime_error("Cannot read map from an Ipe file with more than one page");
	}

	// step 1: find labels
	auto labels = reader.read<Multiple, detail::RegionLabel, WithoutAttributes, RegionLabelReaderTraits>();
	
	// step 2: find regions
	// interpret filled paths as regions
	auto features = reader.read<Multiple, MultipolygonWithHoles<Inexact>, WithAttributes>();

	for (auto& feature : features) {
		auto shape = pretendExact(feature.geometry).polygonSet();
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
			if (feature.attributes.contains("fill")) {
				auto colorString = std::get<std::string>(feature.attributes["fill"]);
				region.color = IpeReader::convertStringToColor(colorString, file);
			}
			region.shape = shape;
			regions[name] = region;
		}
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
}