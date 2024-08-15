#ifndef CARTOCROW_FUNCTION_PAINTING_H
#define CARTOCROW_FUNCTION_PAINTING_H

#include "geometry_renderer.h"
#include "geometry_painting.h"

namespace cartocrow::renderer {
class FunctionPainting : public GeometryPainting {
  public:
	FunctionPainting(const std::function<void(GeometryRenderer&)>& draw_function);
	void paint(renderer::GeometryRenderer& renderer) const override;

  private:
	const std::function<void(GeometryRenderer&)> m_draw_function;
};
}

#endif //CARTOCROW_FUNCTION_PAINTING_H
