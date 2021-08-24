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

#include "svg_visitor.h"

#include <glog/logging.h>

#include "cartocrow/common/circular_range.h"
#include "cartocrow/common/detail/svg_bezier_parser.h"
#include "cartocrow/common/detail/svg_path_parser.h"
#include "cartocrow/common/detail/svg_point_parser.h"
#include "cartocrow/common/detail/svg_polygon_parser.h"

namespace cartocrow {
namespace necklace_map {
namespace detail {
namespace {

constexpr const char* kElementSvg = "svg";

constexpr const char* kAttributeSvgScaleFactor = "scale_factor";
constexpr const char* kAttributeRegionAngle = "angle_rad";
constexpr const char* kAttributeRegionFeasible = "feasible";
constexpr const char* kAttributeRegionId = "region_id";
constexpr const char* kAttributeNecklaceId = "necklace_id";
constexpr const char* kAttributeStyle = "style";
constexpr const char* kAttributeKernelX = "kx";
constexpr const char* kAttributeKernelY = "ky";

constexpr const char* kCommandsRestrictionArcNecklace = "LlZzCcQqSsTt";

} // anonymous namespace

/**@class SvgVisitor
 * @brief An XML visitor for handling SVG necklace map input geometry.
 */

/**@typedef SvgVisitor::NecklaceTypePtr
 * @brief a pointer for a dynamic necklace type.
 */

/**@brief Construct an XML visitor for handling SVG necklace map input geometry.
 * @param regions the collection in which to collect the regions in the input.
 * @param necklaces the collection in which to collect the necklaces in the input.
 * @param scale_factor the scale factor (if defined) or -1 otherwise.
 * @param strict_validity @parblock whether the regions must be strictly valid.
 *
 * Otherwise some regions may be corrected if this will make them valid.
 * @endparblock
 */
SvgVisitor::SvgVisitor(std::vector<necklace_map::MapElement::Ptr>& elements,
                       std::vector<necklace_map::Necklace::Ptr>& necklaces, Number& scale_factor,
                       const bool strict_validity /*= true*/
                       )
    : cartocrow::detail::SvgVisitor(), elements_(elements), necklaces_(necklaces),
      scale_factor_(scale_factor), strict_validity_(strict_validity) {
	// Add the regions to the lookup table, while checking for duplicates.
	for (const MapElement::Ptr& element : elements_) {
		CHECK_NOTNULL(element);
		const size_t next_index = id_to_region_index_.size();
		const size_t n = id_to_region_index_.insert({element->region.id, next_index}).first->second;
		CHECK_EQ(next_index, n);
	}

	necklace_ids_.resize(elements_.size());

	scale_factor_ = -1;
}

bool SvgVisitor::VisitExit(const tinyxml2::XMLElement& element) {
	std::string name = element.Name();
	std::transform(name.begin(), name.end(), name.begin(),
	               [](unsigned char c) { return std::tolower(c); });

	if (name == kElementSvg) {
		return FinalizeSvg();
	}
	return true;
}

void SvgVisitor::VisitSvg(const tinyxml2::XMLAttribute* attributes) {
	std::string scale_factor;
	if (!FindAttribute(attributes, kAttributeSvgScaleFactor, scale_factor)) {
		return;
	}

	try {
		scale_factor_ = std::stod(scale_factor);
	} catch (...) {
		scale_factor_ = -1;
	}
}

bool SvgVisitor::VisitCircle(const Point& center, const Number& radius,
                             const tinyxml2::XMLAttribute* attributes) {
	// Circles without necklace_id are ignored.
	std::string necklace_id;
	if (!FindAttribute(attributes, kAttributeNecklaceId, necklace_id)) {
		return false;
	}

	std::string region_id;
	if (FindAttribute(attributes, kAttributeRegionId, region_id)) {
		CHECK(!region_id.empty());

		// Add a point region.
		std::stringstream stream;
		stream << "M " << center.x() << " " << center.y() << " Z";
		const std::string commands = stream.str();

		std::string style, angle_rad, feasible;
		FindAttribute(attributes, kAttributeStyle, style);
		FindAttribute(attributes, kAttributeRegionAngle, angle_rad);
		FindAttribute(attributes, kAttributeRegionFeasible, feasible);

		return AddMapElement(commands, angle_rad, feasible, region_id, necklace_id, style);
	} else {
		// Add a necklace.
		AddCircleNecklace(necklace_id, center, radius);
		return false;
	}
}

bool SvgVisitor::VisitPath(const std::string& commands, const tinyxml2::XMLAttribute* attributes) {
	CHECK(!commands.empty());

	// Paths without necklace_id are ignored.
	std::string necklace_id;
	if (!FindAttribute(attributes, kAttributeNecklaceId, necklace_id)) {
		return false;
	}

	std::string region_id;
	if (FindAttribute(attributes, kAttributeRegionId, region_id)) {
		CHECK(!region_id.empty());

		// Add a region.
		std::string style, angle_rad, feasible;
		FindAttribute(attributes, kAttributeStyle, style);
		FindAttribute(attributes, kAttributeRegionAngle, angle_rad);
		FindAttribute(attributes, kAttributeRegionFeasible, feasible);

		return AddMapElement(commands, angle_rad, feasible, region_id, necklace_id, style);
	} else {
		// Add a necklace.
		// Necklaces may not contain circular arcs.
		CHECK(commands.find_first_of(kCommandsRestrictionArcNecklace) != std::string::npos);

		const std::string kernel_names[] = {kAttributeKernelX, kAttributeKernelY};
		std::string kernel_position[2];
		CHECK(FindAttributes(attributes, 2, kernel_names, kernel_position));

		try {
			cartocrow::detail::SvgPointParser pp;
			return AddGenericNecklace(necklace_id, commands,
			                          pp.Pt(kernel_position[0], kernel_position[1]));
		} catch (...) {
			return false;
		}
	}
}

/**@brief Connect the regions to their respective necklace.
 * @return whether the SVG was finalized correctly.
 */
bool SvgVisitor::FinalizeSvg() {
	CHECK_EQ(elements_.size(), necklace_ids_.size());
	for (size_t n = 0; n < elements_.size(); ++n) {
		MapElement::Ptr& element = elements_[n];
		CHECK_NOTNULL(element);

		const std::string& necklace_id = necklace_ids_[n];
		if (necklace_id.empty()) {
			continue;
		}

		LookupTable::const_iterator index_iter = id_to_necklace_index_.find(necklace_id);
		CHECK(index_iter != id_to_necklace_index_.end());

		Necklace::Ptr& necklace = necklaces_[index_iter->second];
		element->necklace = necklace;
	}
	return true;
}

/**@brief Add a circle necklace.
 * @param necklace_id the necklace ID.
 * @param center the center of the circle.
 * @param radius the radius of the circle.
 * @return whether the necklace was constructed correctly.
 */
bool SvgVisitor::AddCircleNecklace(const std::string& necklace_id, const Point& center,
                                   const Number& radius) {
	// Necklace IDs must be unique.
	const size_t next_index = necklaces_.size();
	const size_t n = id_to_necklace_index_.insert({necklace_id, next_index}).first->second;
	CHECK_EQ(next_index, n);

	necklaces_.push_back(std::make_shared<necklace_map::Necklace>(
	    necklace_id, std::make_shared<necklace_map::CircleNecklace>(Circle(center, radius * radius))));
	return true;
}

/**@brief Add a generic necklace.
 * @param necklace_id the necklace ID.
 * @param commands the SVG path commands (including point coordinates).
 * @param kernel the kernel of the star-shaped necklace polygon.
 * @return whether the necklace was constructed correctly.
 */
bool SvgVisitor::AddGenericNecklace(const std::string& necklace_id, const std::string& commands,
                                    const Point& kernel) {
	// Necklace IDs must be unique.
	const size_t next_index = necklaces_.size();
	const size_t n = id_to_necklace_index_.insert({necklace_id, next_index}).first->second;
	CHECK_EQ(next_index, n);

	// Interpret the commands as a bezier spline.
	BezierSpline spline;
	cartocrow::detail::SvgBezierConverter converter(spline);
	cartocrow::detail::SvgPathParser()(commands, converter);

	necklace_map::BezierNecklace::Ptr shape =
	    std::make_shared<necklace_map::BezierNecklace>(spline, kernel);
	CHECK(shape->IsValid());

	// Check whether the spline approximates a circle.
	Circle circle;
	const bool is_circle = spline.ToCircle(circle);
	if (is_circle) {
		necklaces_.push_back(std::make_shared<necklace_map::Necklace>(
		    necklace_id, std::make_shared<necklace_map::CircleNecklace>(circle)));
	} else {
		necklaces_.push_back(std::make_shared<necklace_map::Necklace>(necklace_id, shape));
	}
	return true;
}

/**@brief Add a necklace element based on an SVG path.
 * @param commands the SVG path commands (including point coordinates).
 * @param angle_rad the angle of the element's bead, if defined.
 * @param feasible the feasible interval of the element, if defined.
 * @param region_id the ID of the region.
 *
 * See @f Region::Region(const std::string& id) for details on this ID.
 *
 * Note that the ID does not have to unique. If a duplicate ID is encountered, the
 * polygon is added to the region with the same ID.
 * @param necklace_id the ID of the necklace on which to place a bead for this region.
 * @param style the CSS style of the region polygon. Note that this style will be reused
 * for the output regions.
 * @return whether the region could be parsed correctly.
 */
bool SvgVisitor::AddMapElement(const std::string& commands, const std::string& angle_rad,
                               const std::string& feasible, const std::string& region_id,
                               const std::string& necklace_id, const std::string& style) {
	// Get the region with the given ID, or create a new one if it does not yet exist.
	const size_t n = id_to_region_index_.insert({region_id, elements_.size()}).first->second;
	if (n == elements_.size()) {
		elements_.push_back(std::make_shared<necklace_map::MapElement>(region_id));
		necklace_ids_.resize(elements_.size());
	}
	MapElement::Ptr element = elements_[n];
	CHECK_NOTNULL(element);
	Region& region = element->region;
	CHECK_EQ(region_id, region.id);

	// Interpret the commands as a region.
	cartocrow::detail::SvgPolygonConverter converter(region.shape);
	cartocrow::detail::SvgPathParser()(commands, converter);

	region.style = style;
	necklace_ids_[n] = necklace_id;

	if (strict_validity_) {
		CHECK(region.IsValid()) << "Invalid region: " << region.id;
	} else {
		region.MakeValid();
	}

	if (0 <= scale_factor_ && !angle_rad.empty() && !feasible.empty()) {
		try {
			element->input_angle_rad = std::stod(angle_rad);

			std::stringstream stream(feasible);
			Number from_rad, to_rad;
			stream >> from_rad >> to_rad;
			element->input_feasible = std::make_shared<CircularRange>(from_rad, to_rad);
		} catch (...) {
			scale_factor_ = -1;
		}
	}

	return true;
}

} // namespace detail
} // namespace necklace_map
} // namespace cartocrow
