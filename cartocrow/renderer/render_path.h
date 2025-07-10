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
        bool operator==(const MoveTo&) const = default;
        bool operator!=(const MoveTo&) const = default;
	};
	struct LineTo {
		Point<Inexact> m_to;
        bool operator==(const LineTo&) const = default;
        bool operator!=(const LineTo&) const = default;
	};
	struct ArcTo {
		Point<Inexact> m_center;
		bool m_clockwise;
		Point<Inexact> m_to;
        bool operator==(const ArcTo&) const = default;
        bool operator!=(const ArcTo&) const = default;
	};
	struct Close {
        bool operator!=(const Close&) const {
            return false;
        }
        bool operator==(const Close&) const {
            return true;
        }
    };
	using Command = std::variant<MoveTo, LineTo, ArcTo, Close>;

	const std::vector<Command>& commands() const;

    template <class OutputIterator>
    void vertices(OutputIterator out) const {
        for (RenderPath::Command c : m_commands) {
            if (std::holds_alternative<RenderPath::MoveTo>(c)) {
                Point<Inexact> to = std::get<RenderPath::MoveTo>(c).m_to;
                *out++ = to;
            } else if (std::holds_alternative<RenderPath::LineTo>(c)) {
                Point<Inexact> to = std::get<RenderPath::LineTo>(c).m_to;
                *out++ = to;
            } else if (std::holds_alternative<RenderPath::ArcTo>(c)) {
                Point<Inexact> to = std::get<RenderPath::ArcTo>(c).m_to;
                *out++ = to;
            } else if (std::holds_alternative<RenderPath::Close>(c)) {
            } else {
                throw std::runtime_error("Unknown RenderPath command");
            }
        }
    }

	void moveTo(Point<Inexact> to);
	void lineTo(Point<Inexact> to);
	void arcTo(Point<Inexact> center, bool clockwise, Point<Inexact> to);
	void close();

	RenderPath operator+(const RenderPath& other);
	RenderPath& operator+=(const RenderPath& other);

    bool operator==(const RenderPath&) const = default;
    bool operator!=(const RenderPath&) const = default;
  private:
	std::vector<Command> m_commands;
};

/// Apply transformation \p t on render path \p p.
/// \pre Affine transformation \p t should be an orthogonal transformation (no stretching).
renderer::RenderPath orthogonal_transform(const CGAL::Aff_transformation_2<Inexact>& t, const RenderPath& p);
/// Append polygon to render path.
RenderPath& operator<<(RenderPath& path, const Polygon<Inexact>& p);
/// Append polygon with holes to render path.
RenderPath& operator<<(RenderPath& path, const PolygonWithHoles<Inexact>& p);
}

#endif //CARTOCROW_RENDERER_RENDER_PATH_H
