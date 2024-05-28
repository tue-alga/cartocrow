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

#ifndef CARTOCROW_SIMPLE_ISOLINE_PAINTING_H
#define CARTOCROW_SIMPLE_ISOLINE_PAINTING_H

#include "cartocrow/renderer/geometry_painting.h"
#include "isoline.h"
#include "types.h"

namespace cartocrow::isoline_simplification {
class SimpleIsolinePainting : public renderer::GeometryPainting {
  public:
	SimpleIsolinePainting(std::vector<Isoline<K>> isolines);
	void paint(renderer::GeometryRenderer& renderer) const override;

  private:
	std::vector<Isoline<K>> m_isolines;
};
}

#endif //CARTOCROW_SIMPLE_ISOLINE_PAINTING_H
