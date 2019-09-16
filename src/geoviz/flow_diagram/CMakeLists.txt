# The flow_diagram library implements the Flow Diagram geo-visualization method.
set(FLOW_DIAGRAM_TARGET flow_diagram)
set(FLOW_DIAGRAM_TARGET ${FLOW_DIAGRAM_TARGET} PARENT_SCOPE)

file(GLOB_RECURSE HDRS "*.h")
file(GLOB_RECURSE SRCS "*.cpp")

add_library(${FLOW_DIAGRAM_TARGET} ${HDRS} ${SRCS})

# Install rules.
set(INSTALL_FLOW_DIAGRAM_DIR ${GEOVIZ_INSTALL_DIR}/flow_diagram)
install(
    TARGETS ${FLOW_DIAGRAM_TARGET}
    EXPORT ${GEOVIZ_EXPORT_TARGETS}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
)
install_files_and_directories("${HDRS}" ${INSTALL_FLOW_DIAGRAM_DIR})
install_files_and_directories("${SRCS}" ${INSTALL_FLOW_DIAGRAM_DIR})
