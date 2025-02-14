#pragma once

#include "../core/arrangement_map.h"
#include "../core/boundary_map.h"
#include "../core/core.h"
#include "../core/region_map.h"

#include "../renderer/geometry_painting.h"
#include "../renderer/geometry_renderer.h"

using namespace cartocrow;

namespace cartocrow::simplification {

/// The \ref renderer::GeometryPainting "GeometryPainting" for a \ref
/// RegionMap.
class MapPainting : public renderer::GeometryPainting {

  public:
	/// Options that determine what to draw in the painting.
	struct Options {
		/// Default constructor.
		Options(){};
		/// Opacity with which to draw the beads.
		double line_width = 1.0;
		Color color{0, 0, 0};
		bool fill = true;
	};

	/// Creates a new painting for the given region map.
	MapPainting(std::shared_ptr<RegionMap> map, Options options = {})
	    : m_map(map), m_options(options){};

  protected:
	void paint(renderer::GeometryRenderer& renderer) const override {
		if (m_options.fill)
			renderer.setMode(renderer::GeometryRenderer::fill | renderer::GeometryRenderer::stroke);
		else
			renderer.setMode(renderer::GeometryRenderer::stroke);
		renderer.setStroke(m_options.color, m_options.line_width);
		for (auto& it : *m_map) {
			const Region region = it.second;
			if (m_options.fill)
				renderer.setFill(region.color);
			renderer.draw(region.shape);
		}
	}

  private:
	/// The region map we are drawing.
	std::shared_ptr<RegionMap> m_map;
	/// The drawing options.
	Options m_options;
};

/// The \ref renderer::GeometryPainting "GeometryPainting" for a \ref
/// BoundaryMap.
class BoundaryPainting : public renderer::GeometryPainting {

  public:
	/// Options that determine what to draw in the painting.
	struct Options {
		/// Default constructor.
		Options(){};
		/// Opacity with which to draw the beads.
		double line_width = 1.0;
		Color color{0, 0, 0};
	};

	/// Creates a new painting for the given region map.
	BoundaryPainting(std::shared_ptr<BoundaryMap> map, Options options = {})
	    : m_map(map), m_options(options){};

  protected:
	void paint(renderer::GeometryRenderer& renderer) const override {
		renderer.setMode(renderer::GeometryRenderer::stroke);
		renderer.setStroke(m_options.color, m_options.line_width);

		for (Boundary& b : m_map->boundaries) {
			for (int i = 1; i < b.points.size(); i++) {
				Segment<Exact> ls = Segment<Exact>(b.points[i-1], b.points[i]);
				Segment<Inexact> als = approximate(ls);
				renderer.draw(als);
			}
			if (b.closed) {
				Segment<Exact> ls = Segment<Exact>(b.points[b.points.size() - 1], b.points[0]);
				Segment<Inexact> als = approximate(ls);
				renderer.draw(als);
			}
		}
	}

  private:
	/// The region map we are drawing.
	std::shared_ptr<BoundaryMap> m_map;
	/// The drawing options.
	Options m_options;
};

/// The \ref renderer::GeometryPainting "GeometryPainting" for a \ref
/// RegionArrangement.
template <class TArr> class ArrangementPainting : public renderer::GeometryPainting {

  public:
	/// Options that determine what to draw in the painting.
	struct Options {
		/// Default constructor.
		Options(){};
		/// Opacity with which to draw the beads.
		double line_width = 1.0;
		Color color{0, 0, 0};
	};

	/// Creates a new painting for the given arrangement.
	ArrangementPainting(std::shared_ptr<TArr> arr, Options options = {})
	    : m_arr(arr), m_options(options){};

  protected:
	void paint(renderer::GeometryRenderer& renderer) const override {
		renderer.setMode(renderer::GeometryRenderer::stroke);
		renderer.setStroke(m_options.color, m_options.line_width);

		for (auto& e : m_arr->edge_handles()) {
			Segment<Exact> ls = Segment<Exact>(e->source()->point(), e->target()->point());
			Segment<Inexact> als = approximate(ls);
			renderer.draw(als);
		}
	}

  private:
	// The arrangement we are drawing.
	std::shared_ptr<TArr> m_arr;
	// The drawing options.
	Options m_options;
};

} // namespace cartocrow::simplification