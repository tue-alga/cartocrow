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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 09-03-2021
*/

#ifndef CARTOCROW_COMMON_IO_SVG_WRITER_H
#define CARTOCROW_COMMON_IO_SVG_WRITER_H

#include <iostream>
#include <vector>

#include "cartocrow/common/io/write_options.h"
#include "cartocrow/common/polar_line.h"
#include "cartocrow/common/polar_point.h"
#include "cartocrow/common/polar_segment.h"
#include "cartocrow/common/spiral.h"
#include "cartocrow/common/spiral_segment.h"

namespace cartocrow {

class SvgWriter {
  public:
	SvgWriter();

	void Add(const PolarPoint& point);

	void Add(const Spiral& spiral);

	void Add(const SpiralSegment& segment);

	void Add(const PolarLine& line);

	void Add(const PolarSegment& segment);

	bool Write(const WriteOptions::Ptr& options, std::ostream& out) const;

  private:
	std::vector<PolarPoint> points_;
	std::vector<Spiral> spirals_;
	std::vector<SpiralSegment> spiral_segments_;
	std::vector<PolarLine> lines_;
	std::vector<PolarSegment> line_segments_;
}; // class SvgWriter

} // namespace cartocrow

#endif //CARTOCROW_COMMON_IO_SVG_WRITER_H
