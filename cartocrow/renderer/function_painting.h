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

#pragma once

#include "geometry_renderer.h"
#include "geometry_painting.h"

namespace cartocrow {
class FunctionPainting : public GeometryPainting {
  public:
	FunctionPainting(const std::function<void(GeometryRenderer&)>& draw_function);
	void paint(GeometryRenderer& renderer) const override;

  private:
	const std::function<void(GeometryRenderer&)> m_draw_function;
};
}
