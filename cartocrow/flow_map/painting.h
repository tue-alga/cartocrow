#ifndef CARTOCROW_FLOW_MAP_PAINTING
#define CARTOCROW_FLOW_MAP_PAINTING

#include "../renderer/geometry_painting.h"
#include "../renderer/geometry_renderer.h"
#include "spiral.h"

#include "spiral_tree.h"

namespace cartocrow::flow_map {

/// The \ref renderer::GeometryPainting "GeometryPainting" for a spiral tree.
class Painting : public renderer::GeometryPainting {

  public:
	/// Options that determine what to draw in the painting.
	struct Options {
		double spiralStep = 0.01;
		double spiralMax = 6.0;
	};

	/// Creates a new painting with the given map and tree.
	Painting(std::shared_ptr<RegionMap> map, std::shared_ptr<SpiralTree> tree, const Options options);

  protected:
	void paint(renderer::GeometryRenderer& renderer) const override;

  private:
	void paintRegions(renderer::GeometryRenderer& renderer) const;
	void paintObstacles(renderer::GeometryRenderer& renderer) const;
	void paintNodes(renderer::GeometryRenderer& renderer) const;
	void paintFlow(renderer::GeometryRenderer& renderer) const;

	void paintSpiral(renderer::GeometryRenderer& renderer, const Spiral& spiral,
	                 const Point<Inexact>& offset, const PolarPoint& parent) const;

	std::shared_ptr<RegionMap> m_map;
	std::shared_ptr<SpiralTree> m_tree;
	Options m_options;
};

} // namespace cartocrow::flow_map

#endif //CARTOCROW_FLOW_MAP_PAINTING
