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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 02-09-2020
*/

#ifndef CARTOCROW_COMMON_IO_WRITE_OPTIONS_H
#define CARTOCROW_COMMON_IO_WRITE_OPTIONS_H

#include <memory>


namespace cartocrow
{

struct WriteOptions
{
  using Ptr = std::shared_ptr<WriteOptions>;

  static Ptr Default();

  static Ptr Debug();

  int pixel_width;
  int numeric_precision;
}; // struct WriteOptions

} // namespace cartocrow

#endif //CARTOCROW_COMMON_IO_WRITE_OPTIONS_H
