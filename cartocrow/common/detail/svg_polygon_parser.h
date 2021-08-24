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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 20-11-2019
*/

#ifndef CARTOCROW_COMMON_DETAIL_SVG_POLYGON_PARSER_H
#define CARTOCROW_COMMON_DETAIL_SVG_POLYGON_PARSER_H

#include "cartocrow/common/core_types.h"

#include "cartocrow/common/detail/svg_path_parser.h"

namespace cartocrow {
namespace detail {

class SvgPolygonConverter : public SvgPathConverter {
  public:
	using PolygonSet = std::vector<Polygon_with_holes>;

  public:
	explicit SvgPolygonConverter(PolygonSet& shape);

  private:
	void MoveTo_(const Point& to) override;
	void LineTo_(const Point& to) override;
	void Close_() override;

	PolygonSet& shape_;
	Polygon current_;
}; // class SvgPolygonConverter

} // namespace detail
} // namespace cartocrow

#endif //CARTOCROW_COMMON_DETAIL_SVG_POLYGON_PARSER_H
