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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 29-01-2020
*/

#ifndef CARTOCROW_NECKLACE_MAP_IO_DETAIL_SVG_WRITER_H
#define CARTOCROW_NECKLACE_MAP_IO_DETAIL_SVG_WRITER_H

#include <string>
#include <unordered_map>
#include <vector>

#include <tinyxml2.h>

#include "cartocrow/core/core_types.h"
#include "cartocrow/necklace_map/io/write_options.h"
#include "cartocrow/necklace_map/map_element.h"
#include "cartocrow/necklace_map/necklace.h"

namespace cartocrow {
namespace necklace_map {
namespace detail {

class SvgWriter {
  public:
	using MapElement = necklace_map::MapElement;
	using Necklace = necklace_map::Necklace;

	SvgWriter(const std::vector<MapElement::Ptr>& elements,
	          const std::vector<Necklace::Ptr>& necklaces, const Number& scale_factor,
	          const WriteOptions::Ptr& options, std::ostream& out);

	~SvgWriter();

	void DrawPolygonRegions();

	void DrawPointRegions();

	void DrawNecklaces();

	void DrawBeads();

	void DrawFeasibleIntervals();

	void DrawValidIntervals();

	void DrawRegionAngles();

	void DrawBeadAngles();

  private:
	using Bead = necklace_map::Bead;

	using NecklaceShape = necklace_map::NecklaceShape;
	using CircleNecklace = necklace_map::CircleNecklace;
	using BezierNecklace = necklace_map::BezierNecklace;
	using BeadIntervalMap = std::unordered_map<Bead::Ptr, CircleNecklace::Ptr>;

	void OpenSvg();

	void CloseSvg();

	void ComputeBoundingBox();

	void CreateBeadIntervalShapes();

	void AddDropShadowFilter();

	void DrawKernel(const Point& kernel);

	void DrawBeadIds();

	const std::vector<MapElement::Ptr>& elements_;
	const std::vector<Necklace::Ptr>& necklaces_;
	const Number scale_factor_;
	std::ostream& out_;

	WriteOptions::Ptr options_;

	Box bounding_box_;
	double unit_px_;
	std::string transform_matrix_;

	BeadIntervalMap bead_interval_map_;

	tinyxml2::XMLPrinter printer_;
}; // class SvgWriter

} // namespace detail
} // namespace necklace_map
} // namespace cartocrow

#endif //CARTOCROW_NECKLACE_MAP_IO_DETAIL_SVG_WRITER_H
