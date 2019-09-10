# The flow_diagram library implements the Flow Diagram geo-visualization method.
set(TARGET_LIB_FLOW_DIAGRAM flow_diagram)
set(TARGET_LIB_FLOW_DIAGRAM ${TARGET_LIB_FLOW_DIAGRAM} PARENT_SCOPE)

file(GLOB_RECURSE HDRS "*.h")
file(GLOB_RECURSE SRCS "*.cpp")

add_library(${TARGET_LIB_FLOW_DIAGRAM} ${HDRS} ${SRCS})

# Install rules.
install(TARGETS ${TARGET_LIB_FLOW_DIAGRAM} DESTINATION ${CMAKE_INSTALL_BINARY_DIR})
install(FILES ${HDRS} ${SRCS} DESTINATION ${INSTALL_GEOVIZ_DIR}/flow_diagram)
