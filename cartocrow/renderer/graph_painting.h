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

#include "geometry_painting.h"

namespace cartocrow::renderer {

enum VertexSelection { DEG0_ONLY = 0, NO_DEG2 = 1, ALL = 2 };

struct VertexStyle {
	inline static const Color M_DEFAULT_COLOR = {0, 0, 0};
	static constexpr const double M_DEFAULT_SIZE = 6;

	const Color m_color;
	const double m_size;

	VertexStyle(Color color = {0, 0, 0}, double size = 6)
	    : m_color(color), m_size(size) {}
};

struct EdgeStyle {

	const Color m_color;
	const double m_linewidth;

	EdgeStyle(Color color = {0, 0, 0}, double linewidth = 1)
	    : m_color(color), m_linewidth(linewidth) {}
};

template <class Graph> class GraphPainting : public GeometryPainting {
	  public:
		GraphPainting(std::shared_ptr<Graph> graph,
		              const std::optional<VertexStyle> vertex_style = VertexStyle(),
		              const std::optional<EdgeStyle> edge_style = EdgeStyle(), const VertexSelection vertex_selection = ALL)
	        : m_graph(std::move(graph)), m_vertex_style(vertex_style), m_edge_style(edge_style),
	          m_vertex_selection(vertex_selection) {}

	  private:
		const std::shared_ptr<Graph> m_graph;
		const std::optional<VertexStyle> m_vertex_style;
		const std::optional<EdgeStyle> m_edge_style;
	    const VertexSelection m_vertex_selection;

	  protected:
		void defaultVertexStyle(GeometryRenderer & renderer) const {
			renderer.setMode(GeometryRenderer::fill);
			renderer.setStroke(m_vertex_style->m_color, 1);
			renderer.setPointSize(m_vertex_style->m_size);
		}

		virtual void drawVertex(GeometryRenderer & renderer,
		                        typename Graph::Vertex_const_handle vertex) const {
			switch (m_vertex_selection) {
			case DEG0_ONLY:
				if (vertex->degree() != 0)
					return;
			case NO_DEG2:
				if (vertex->degree() == 2)
					return;
			}

			renderer.draw(vertex->point());
		}

		void defaultEdgeStyle(GeometryRenderer & renderer) const {
			renderer.setMode(GeometryRenderer::stroke);
			renderer.setStroke(m_edge_style->m_color, m_edge_style->m_linewidth);
		}

		/// Applies a custom style for the given edge (or otherwise, maintains the current/default style). Call defaultEdgeStyle to switch between custom and default styling.
		/// The return value is used to determine whether the edge is to be drawn at all.
		virtual void drawEdge(GeometryRenderer & renderer, typename Graph::Edge_const_handle edge)
		    const {
			renderer.draw(edge->curve());
		}

		void paint(GeometryRenderer & renderer) const override {
			if (m_edge_style) {
				defaultEdgeStyle(renderer);
				for (typename Graph::Edge_const_handle e : m_graph->edges()) {
					drawEdge(renderer, e);
				}
			}

			if (m_vertex_style) {
				defaultVertexStyle(renderer);
				for (typename Graph::Vertex_const_handle v : m_graph->vertices()) {
					drawVertex(renderer, v);
				}
			}
		}
};
} // namespace cartocrow::renderer