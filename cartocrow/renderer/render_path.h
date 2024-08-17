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

#ifndef CARTOCROW_RENDERER_RENDER_PATH_H
#define CARTOCROW_RENDERER_RENDER_PATH_H

#include <variant>

#include "../core/core.h"

namespace cartocrow::renderer {

/// A path that can be drawn or filled.
class RenderPath {
  public:
	struct MoveTo {
		Point<Inexact> m_to;
	};
	struct LineTo {
		Point<Inexact> m_to;
	};
	struct ArcTo {
		Point<Inexact> m_center;
		bool m_clockwise;
		Point<Inexact> m_to;
	};
	struct Close {};
	using Command = std::variant<MoveTo, LineTo, ArcTo, Close>;

	const std::vector<Command>& commands() const;

	void moveTo(Point<Inexact> to);
	void lineTo(Point<Inexact> to);
	void arcTo(Point<Inexact> center, bool clockwise, Point<Inexact> to);
	void close();

  private:
	std::vector<Command> m_commands;
};

}

#endif //CARTOCROW_RENDERER_RENDER_PATH_H
