# The flow_diagram library implements the Flow Diagram geo-visualization method.
set(FLOW_DIAGRAM_TARGET flow_diagram)

geoviz_add_library(${FLOW_DIAGRAM_TARGET} ${GEOVIZ_INSTALL_DIR}/${FLOW_DIAGRAM_TARGET})

set(FLOW_DIAGRAM_TARGET ${FLOW_DIAGRAM_TARGET} PARENT_SCOPE)
