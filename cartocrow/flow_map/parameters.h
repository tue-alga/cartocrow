/*
The Flow Map library implements the algorithmic geo-visualization
method by the same name, developed by Kevin Verbeek, Kevin Buchin,
and Bettina Speckmann at TU Eindhoven
(DOI: 10.1007/s00453-013-9867-z & 10.1109/TVCG.2011.202).
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

#ifndef CARTOCROW_FLOW_MAP_PARAMETERS_H
#define CARTOCROW_FLOW_MAP_PARAMETERS_H

#include "cartocrow/common/core_types.h"

namespace cartocrow {
namespace flow_map {

struct Parameters {
	Parameters();

	Number restricting_angle_rad;
}; // struct Parameters

} // namespace flow_map
} // namespace cartocrow

#endif //CARTOCROW_FLOW_MAP_PARAMETERS_H
