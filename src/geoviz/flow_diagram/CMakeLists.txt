# The flow_diagram library implements the Flow Diagram geo-visualization method.
set(TARGET_LIB_FLOW_DIAGRAM flow_diagram)
set(TARGET_LIB_FLOW_DIAGRAM ${TARGET_LIB_FLOW_DIAGRAM} PARENT_SCOPE)

file(GLOB_RECURSE HDRS "*.h")
file(GLOB_RECURSE SRCS "*.cpp")

add_library(${TARGET_LIB_FLOW_DIAGRAM} ${HDRS} ${SRCS})

# Install rules.
set(INSTALL_FLOW_DIAGRAM_DIR ${INSTALL_GEOVIZ_DIR}/flow_diagram)
install(
    TARGETS ${TARGET_LIB_FLOW_DIAGRAM}
    EXPORT ${INSTALL_GEOVIZ_EXPORT}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
)
install_files_and_directories("${HDRS}" ${INSTALL_FLOW_DIAGRAM_DIR})
install_files_and_directories("${SRCS}" ${INSTALL_FLOW_DIAGRAM_DIR})
