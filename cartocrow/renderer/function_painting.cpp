#include "function_painting.h"

namespace cartocrow::renderer {
FunctionPainting::FunctionPainting(const std::function<void(GeometryRenderer&)>& draw_function)
    : m_draw_function(draw_function) {}

void FunctionPainting::paint(GeometryRenderer& renderer) const {
	m_draw_function(renderer);
}
}