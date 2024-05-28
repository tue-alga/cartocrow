/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
Copyright (C) 2024 TU Eindhoven

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

#include "simple_isoline_painting.h"

namespace cartocrow::isoline_simplification {
SimpleIsolinePainting::SimpleIsolinePainting(std::vector<Isoline<K>> isolines)
    : m_isolines(std::move(isolines)) {}

void SimpleIsolinePainting::paint(renderer::GeometryRenderer& renderer) const {
	renderer.setMode(renderer::GeometryRenderer::stroke);
	renderer.setStroke(Color{0, 0, 0}, 1.0);

	for (const auto& isoline : m_isolines) {
		std::visit([&](auto&& v) { renderer.draw(v); }, isoline.drawing_representation());
	}
}
}
