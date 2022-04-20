#ifndef CARTOCROW_NECKLACE_MAP_PAINTING
#define CARTOCROW_NECKLACE_MAP_PAINTING

#include <cartocrow/core/spiral.h>
#include <cartocrow/renderer/geometry_painting.h>
#include <cartocrow/renderer/geometry_renderer.h>

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

	/// Creates a new painting with the given tree, regions, and obstacles.
	Painting(const SpiralTree& tree, const std::vector<Region>& regions,
	         const std::vector<Region>& obstacles, const Options options);

  protected:
	void paint(renderer::GeometryRenderer& renderer) override;

  private:
	void paintRegions(renderer::GeometryRenderer& renderer);
	void paintObstacles(renderer::GeometryRenderer& renderer);
	void paintNodes(renderer::GeometryRenderer& renderer);
	void paintFlow(renderer::GeometryRenderer& renderer);

	void paintSpiral(renderer::GeometryRenderer& renderer, const Spiral& spiral,
	                 const Point& offset, const PolarPoint& parent);

	const SpiralTree& m_tree;
	const std::vector<Region>& m_regions;
	const std::vector<Region>& m_obstacles;
	Options m_options;
};

} // namespace cartocrow::flow_map

#endif //CARTOCROW_NECKLACE_MAP_PAINTING
