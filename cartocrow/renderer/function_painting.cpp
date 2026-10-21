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

#include "function_painting.h"

namespace cartocrow {
FunctionPainting::FunctionPainting(const std::function<void(GeometryRenderer&)>& draw_function)
    : m_draw_function(draw_function) {}

void FunctionPainting::paint(GeometryRenderer& renderer) const {
	m_draw_function(renderer);
}
}