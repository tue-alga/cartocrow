/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
Copyright (C) 2024 TU Eindhoven

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3f of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef CARTOCROW_VORONOI_DRAWER_H
#define CARTOCROW_VORONOI_DRAWER_H

#include "cartocrow/renderer/geometry_renderer.h"
#include <CGAL/Parabola_segment_2.h>

template < class Gt >
class VoronoiDrawer {
  public:
	cartocrow::renderer::GeometryRenderer* m_renderer;

	explicit VoronoiDrawer(cartocrow::renderer::GeometryRenderer* renderer): m_renderer(renderer) {};

	VoronoiDrawer& operator<<(const Gt::Segment_2& s) {
		m_renderer->draw(s);
		return *this;
	}

	VoronoiDrawer& operator<<(const Gt::Line_2& l) {
		m_renderer->draw(l);
		return *this;
	}

	VoronoiDrawer& operator<<(const Gt::Ray_2& r){
		m_renderer->draw(r);
		return *this;
	}

	VoronoiDrawer& operator<<(const CGAL::Parabola_segment_2<Gt>& p){
		// Directrix
		auto dir = p.line();
		// Focus
		auto focus = p.center();

		// Roundabout way to obtain start and end of parabolic segment because they are protected -_-
		std::vector<typename Gt::Point_2> pts;
		p.generate_points(pts, (typename Gt::FT)(1000000));
		auto start = pts.front();
		auto end = pts.back();

		// Geometric magic: the intersection of the tangents at points p and q of the parabola is
		// the circumcenter of the focus and the projections of p and q on the directrix.
		auto start_p = dir.projection(start);
		auto end_p = dir.projection(end);

		// If the points are collinear CGAL::circumcenter throws an error; draw a segment instead.
		if (CGAL::collinear(focus, start_p, end_p)) return *this << typename Gt::Segment_2(start, end);

		auto control = CGAL::circumcenter(focus, start_p, end_p);
		auto bezier = BezierCurve(approximate(start), approximate(control), approximate(end));

		m_renderer->draw(bezier);

		return *this;
	}
};

#endif //CARTOCROW_VORONOI_DRAWER_H
