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

#include "render_path.h"

namespace cartocrow::renderer {

const std::vector<RenderPath::Command>& RenderPath::commands() const {
	return m_commands;
}

void RenderPath::moveTo(Point<Inexact> to) {
	m_commands.push_back(MoveTo{to});
}

void RenderPath::lineTo(Point<Inexact> to) {
	m_commands.push_back(LineTo{to});
}

void RenderPath::arcTo(Point<Inexact> center, bool clockwise, Point<Inexact> to) {
	m_commands.push_back(ArcTo{center, clockwise, to});
}

void RenderPath::close() {
	m_commands.push_back(Close{});
}

} // namespace cartocrow::renderer
