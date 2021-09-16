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

#include <glog/logging.h>

#include <ipeattributes.h>
#include <ipebase.h>
#include <ipelib.h>

#include <fstream>
#include <memory>
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
	for (int i = 0; i < page->count(); ++i) {
		ipe::Object* object = page->object(i);
		ipe::Object::Type type = object->type();
		if (type != ipe::Object::Type::EPath) {
			continue;
		}
		ipe::Path* path = object->asPath();
		ipe::Shape shape = path->shape();
		// interpret stroked paths as necklaces; others are considered regions
		if (path->pathMode() == ipe::TPathMode::EStrokedOnly) {
			auto necklace = std::make_shared<Necklace>(
			    "necklace",
			    std::make_shared<CircleNecklace>(Circle(Point(500, 200), Point(2000, 200)))); // TODO
			necklaces.push_back(necklace);
		} else {
			auto element = std::make_shared<necklace_map::MapElement>(std::to_string(i));
			Polygon p; // TODO
			p.push_back(Point(200 + 100 * i, 200));
			p.push_back(Point(300 + 100 * i, 300));
			p.push_back(Point(250 + 100 * i, 350));
			Polygon_with_holes polygon(p);
			element->region.shape.push_back(polygon);
			elements.push_back(element);
		}
	}

	// TODO: temporarily assign everything to the first necklace
	for (auto element : elements) {
		element->necklace = necklaces[0];
	}

	std::cout << "Done reading!" << std::endl;

	return true;
}

} // namespace cartocrow::necklace_map
