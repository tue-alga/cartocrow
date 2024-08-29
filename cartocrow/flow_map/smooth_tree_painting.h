#ifndef CARTOCROW_FLOW_MAP_SMOOTH_TREE_PAINTING
#define CARTOCROW_FLOW_MAP_SMOOTH_TREE_PAINTING

#include "../renderer/geometry_painting.h"
#include "../renderer/geometry_renderer.h"
#include "spiral.h"

#include "smooth_tree.h"

namespace cartocrow::flow_map {

/// The \ref renderer::GeometryPainting "GeometryPainting" for a \ref SmoothTree.
class SmoothTreePainting : public renderer::GeometryPainting {

  public:
	/// Options that determine what to draw in the painting.
	struct Options {
		double spiralStep = 0.01;
		double spiralMax = 6.0;
	};

	/// Creates a new painting with the given map and tree.
	SmoothTreePainting(std::shared_ptr<SmoothTree> tree, const Options options);

  protected:
	void paint(renderer::GeometryRenderer& renderer) const override;

  private:
	void paintNodes(renderer::GeometryRenderer& renderer) const;
	void paintFlow(renderer::GeometryRenderer& renderer) const;

	std::shared_ptr<SmoothTree> m_tree;
	Options m_options;
};

} // namespace cartocrow::flow_map

#endif //CARTOCROW_FLOW_MAP_PAINTING
