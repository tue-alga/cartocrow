#pragma once

#include "../core/core.h"
#include "../core/region_arrangement.h"

#include "../renderer/geometry_painting.h"
#include "../renderer/geometry_renderer.h"

namespace cartocrow::simplification {

/// The \ref renderer::GeometryPainting "GeometryPainting" for a \ref
/// NecklaceMap.
template <class TArr> class Painting : public renderer::GeometryPainting {

  public:
	/// Options that determine what to draw in the painting.
	struct Options {
		/// Default constructor.
		Options(){};
		/// Opacity with which to draw the beads.
		double line_width = 1.0;
		Color color{0, 0, 0};
	};

	/// Creates a new painting for the given necklace map.
	Painting(std::shared_ptr<TArr> arr, Options options = {});

  protected:
	void paint(renderer::GeometryRenderer& renderer) const override;

  private:
	/// The necklace map we are drawing.
	std::shared_ptr<TArr> m_arr;
	/// The drawing options.
	Options m_options;
};

template <class TArr>
Painting<TArr>::Painting(std::shared_ptr<TArr> arr, Options options)
    : m_arr(arr), m_options(options) {}

template <class TArr> void Painting<TArr>::paint(renderer::GeometryRenderer& renderer) const {
	renderer.setMode(renderer::GeometryRenderer::stroke);
	renderer.setStroke(m_options.color, m_options.line_width);

	for (auto e : m_arr->halfedge_handles()) {
		Segment<Exact> ls = Segment<Exact>(e->source()->point(), e->target()->point());
		Segment<Inexact> als = approximate(ls);
		renderer.draw(als);
	}
}

} // namespace cartocrow::simplification