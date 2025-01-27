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

#include "geometry_renderer.h"
#include <CGAL/number_utils.h>

namespace cartocrow::renderer {

void GeometryRenderer::draw(const Segment<Inexact>& s) {
	RenderPath path;
	path.moveTo(s.start());
	path.lineTo(s.end());
	draw(path);
}

void GeometryRenderer::draw(const Rectangle<Inexact>& r) {
	RenderPath path;
	path.moveTo(r.vertex(0));
	for (int i = 1; i < 4; ++i) {
		path.lineTo(r.vertex(i));
	}
	path.close();
	draw(path);
}

void GeometryRenderer::draw(const Triangle<Inexact>& t) {
	RenderPath path;
	path.moveTo(t.vertex(0));
	for (int i = 1; i < 3; ++i) {
		path.lineTo(t.vertex(i));
	}
	path.close();
	draw(path);
}

void GeometryRenderer::draw(const Box& b) {
	draw(Rectangle<Inexact>({b.xmin(), b.ymin()}, {b.xmax(), b.ymax()}));
}

void GeometryRenderer::draw(const Polygon<Inexact>& p) {
	RenderPath path;
	for (auto vertex = p.vertices_begin(); vertex != p.vertices_end(); vertex++) {
		if (vertex == p.vertices_begin()) {
			path.moveTo(*vertex);
		} else {
			path.lineTo(*vertex);
		}
	}
	path.close();
	draw(path);
}

void GeometryRenderer::draw(const Polyline<Inexact>& p) {
	RenderPath path;
	for (auto vertex = p.vertices_begin(); vertex != p.vertices_end(); vertex++) {
		if (vertex == p.vertices_begin()) {
			path.moveTo(*vertex);
		} else {
			path.lineTo(*vertex);
		}
	}
	draw(path);
}

void GeometryRenderer::draw(const PolygonWithHoles<Inexact>& p) {
	RenderPath path;
	path << p;
	draw(path);
}

void GeometryRenderer::draw(const PolygonSet<Inexact>& ps) {
	std::vector<PolygonWithHoles<Inexact>> polygons;
	ps.polygons_with_holes(std::back_inserter(polygons));
	RenderPath path;
	for (const auto& p : polygons) {
		path << p;
	}
	draw(path);
}

void GeometryRenderer::draw(const BezierCurve& c) {
	BezierSpline spline;
	spline.appendCurve(c);
	draw(spline);
}

} // namespace cartocrow::renderer
