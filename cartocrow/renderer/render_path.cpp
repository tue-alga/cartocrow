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

RenderPath RenderPath::operator+(const RenderPath& other) {
	RenderPath path;
	std::copy(m_commands.begin(), m_commands.end(), std::back_inserter(path.m_commands));
	std::copy(other.m_commands.begin(), other.m_commands.end(), std::back_inserter(path.m_commands));
	return path;
}

RenderPath& RenderPath::operator+=(const RenderPath& other) {
	std::copy(other.m_commands.begin(), other.m_commands.end(), std::back_inserter(m_commands));
	return *this;
}

renderer::RenderPath orthogonal_transform(const CGAL::Aff_transformation_2<Inexact>& t, const RenderPath& p) {
    RenderPath tp;
    for (auto& cmd : p.commands()) {
        if (auto* c = std::get_if<RenderPath::MoveTo>(&cmd)) {
            tp.moveTo(c->m_to.transform(t));
        } else if (auto* c = std::get_if<RenderPath::LineTo>(&cmd)) {
            tp.lineTo(c->m_to.transform(t));
        } else if (auto* c = std::get_if<RenderPath::ArcTo>(&cmd)) {
            tp.arcTo(c->m_center.transform(t), t.is_reflection() ? !c->m_clockwise : c->m_clockwise, c->m_to.transform(t));
        } else if (auto* c = std::get_if<RenderPath::Close>(&cmd)) {
            tp.close();
        } else {
            throw std::runtime_error("Unknown render path command");
        }
    }
    return tp;
}

RenderPath& operator<<(RenderPath& path, const Polygon<Inexact>& p) {
	for (auto vertex = p.vertices_begin(); vertex != p.vertices_end(); vertex++) {
		if (vertex == p.vertices_begin()) {
			path.moveTo(*vertex);
		} else {
			path.lineTo(*vertex);
		}
	}
	path.close();
	return path;
}

RenderPath& operator<<(RenderPath& path, const PolygonWithHoles<Inexact>& p) {
	path << p.outer_boundary();
	for (const auto& h : p.holes()) {
		path << h;
	}
	return path;
}

} // namespace cartocrow::renderer
