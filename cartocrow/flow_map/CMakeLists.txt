# The flow_map library implements the Flow Map geo-visualization method.
set(FLOW_MAP_TARGET flow_map)

cartocrow_add_library(${FLOW_MAP_TARGET} ${CARTOCROW_INSTALL_DIR}/${FLOW_MAP_TARGET})

set(FLOW_MAP_TARGET ${FLOW_MAP_TARGET} PARENT_SCOPE)