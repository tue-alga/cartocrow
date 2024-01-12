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

#ifndef CARTOCROW_CORE_IO_IPE_READER_H
#define CARTOCROW_CORE_IO_IPE_READER_H

#include <filesystem>
#include <memory>

#include "core.h"
#include "bezier.h"

#include <ipeattributes.h>
#include <ipedoc.h>
#include <ipeshape.h>

namespace cartocrow {

/// Various utility methods for reading Ipe files.
class IpeReader {
  public:
	/// Loads the given Ipe file into an Ipe document.
	/**
	 * This encapsulates the things necessary in Ipelib to load from the file.
     * It throws an exception if the file could not be read correctly.
	 */
	static std::shared_ptr<ipe::Document> loadIpeFile(const std::filesystem::path& filename);
	/// Converts an Ipe color to a CartoCrow color.
	static Color convertIpeColor(ipe::Color color);
	/// Converts an Ipe shape to a polygon set.
	/**
	 * Throws if the shape contains a non-polygonal boundary.
	 */
	static PolygonSet<Exact> convertShapeToPolygonSet(const ipe::Shape& shape,
	                                                  const ipe::Matrix& matrix);
	/// Converts an Ipe path to a Bézier spline.
	static BezierSpline convertPathToSpline(const ipe::SubPath& path, const ipe::Matrix& matrix);
};

} // namespace cartocrow

#endif //CARTOCROW_CORE_IO_IPE_READER_H
