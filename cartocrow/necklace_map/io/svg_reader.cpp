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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 26-11-2019
*/

#include "svg_reader.h"

#include <fstream>

#include <glog/logging.h>

#include "cartocrow/necklace_map/io/detail/svg_visitor.h"

namespace cartocrow {
namespace necklace_map {

/**@class SvgReader
 * @brief A reader for SVG necklace map input geometry.
 */

/**@brief Construct a reader for SVG necklace map input geometry.
 */
SvgReader::SvgReader() {}

/**@brief Read necklace map SVG input from a file.
 * @param filename the file to read.
 * @param elements the collection in which to collect the regions in the input.
 * @param necklaces where to place the necklaces.
 * @param max_retries the maximum number of times to retry reading the file.
 * @return whether the read operation could be completed successfully.
 */
bool SvgReader::ReadFile(const std::filesystem::path& filename,
                         std::vector<necklace_map::MapElement::Ptr>& elements,
                         std::vector<necklace_map::Necklace::Ptr>& necklaces, int max_retries /*= 2*/
) {
	Number scale_factor;
	return ReadFile(filename, elements, necklaces, scale_factor, max_retries);
}

/**@brief Read necklace map SVG input from a file.
 * @param filename the file to read.
 * @param elements the collection in which to collect the regions in the input.
 * @param necklaces where to place the necklaces.
 * @param scale_factor where to place the scale factor, if defined.
 * @param max_retries the maximum number of times to retry reading the file.
 * @return whether the read operation could be completed successfully.
 */
bool SvgReader::ReadFile(const std::filesystem::path& filename,
                         std::vector<necklace_map::MapElement::Ptr>& elements,
                         std::vector<necklace_map::Necklace::Ptr>& necklaces, Number& scale_factor,
                         int max_retries /*= 2*/
) {
	std::string input;
	int retry = 0;
	do {
		try {
			std::fstream fin(filename);
			if (fin) {
				using Iterator = std::istreambuf_iterator<char>;
				input.assign(Iterator(fin), Iterator());
				break;
			}
		} catch (const std::exception& e) {
			LOG(ERROR) << e.what();
		}

		if (max_retries < retry++) {
			LOG(INFO) << "Failed to open necklace map geometry file: " << filename;
			return false;
		}
	} while (true);

	return Parse(input, elements, necklaces, scale_factor);
}

/**@brief Parse necklace map SVG input from a string.
 * @param input the string to parse.
 * @param elements the collection in which to collect the regions in the input.
 * @param necklaces where to place the necklaces.
 * @param scale_factor where to place the scale factor, if defined.
 * @return whether the string could be parsed successfully.
 */
bool SvgReader::Parse(const std::string& input, std::vector<necklace_map::MapElement::Ptr>& elements,
                      std::vector<necklace_map::Necklace::Ptr>& necklaces, Number& scale_factor) {
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError result = doc.Parse(input.data());
	//tinyxml2::XMLError result = doc.Parse(content_str); // This would be how to parse a string instead of a file using tinyxml2
	if (result != tinyxml2::XML_SUCCESS) {
		return false;
	}

	using Visitor = detail::SvgVisitor;
	Visitor visitor(elements, necklaces, scale_factor);
	doc.Accept(&visitor);

	// Note(tvl) we should allow the SVG to not contain the necklace: then create the necklace as smallest enclosing circle.
	LOG(INFO) << "Successfully parsed necklace map geometry for " << elements.size()
	          << " region(s) and " << necklaces.size() << " necklace(s).";

	return true;
}

} // namespace necklace_map
} // namespace cartocrow
