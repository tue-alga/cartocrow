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
#include <CGAL/Parabola_segment_2.h>

#include "cartocrow/core/segment_delaunay_graph_helpers.h"

namespace cartocrow {
/// This class can be used in combination with the draw_dual and draw_dual_edge functions of
/// CGAL's Voronoi and Segment Voronoi diagrams.
/// Also see the modified draw_dual function for segment Voronoi diagrams in "segment_delaunay_graph_helpers.h"
template < class Gt >
class VoronoiDrawer {
  public:
	using GtE = CGAL::Segment_Delaunay_graph_traits_2<Exact>;

	cartocrow::GeometryRenderer* m_renderer;

	explicit VoronoiDrawer(cartocrow::GeometryRenderer* renderer): m_renderer(renderer) {};

	VoronoiDrawer& operator<<(const typename Gt::Segment_2& s) {
		m_renderer->draw(s);
		return *this;
	}

	VoronoiDrawer& operator<<(const typename Gt::Line_2& l) {
		m_renderer->draw(l);
		return *this;
	}

	VoronoiDrawer& operator<<(const typename Gt::Ray_2& r){
		m_renderer->draw(r);
		return *this;
	}

	VoronoiDrawer& operator<<(const typename CGAL::Parabola_segment_2<Gt>& p){
		m_renderer->draw(parabolaSegmentToBezier(p));
		return *this;
	}

	VoronoiDrawer& operator<<(const typename GtE::Segment_2& s) {
		m_renderer->draw(s);
		return *this;
	}

	VoronoiDrawer& operator<<(const typename GtE::Line_2& l) {
		m_renderer->draw(l);
		return *this;
	}

	VoronoiDrawer& operator<<(const typename GtE::Ray_2& r){
		m_renderer->draw(r);
		return *this;
	}

	VoronoiDrawer& operator<<(const typename CGAL::Parabola_segment_2<GtE>& p){
		m_renderer->draw(parabolaSegmentToBezier(p));
		return *this;
	}
};
}
