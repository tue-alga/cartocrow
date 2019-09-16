# The necklace_map library implements the Necklace Map geo-visualization method.
set(NECKLACE_MAP_TARGET necklace_map)
set(NECKLACE_MAP_TARGET ${NECKLACE_MAP_TARGET} PARENT_SCOPE)

file(GLOB_RECURSE HDRS "*.h")
file(GLOB_RECURSE SRCS "*.cpp")

add_library(${NECKLACE_MAP_TARGET} ${HDRS} ${SRCS})

# Install rules.
set(INSTALL_NECKLACE_MAP_DIR ${GEOVIZ_INSTALL_DIR}/necklace_map)
install(
    TARGETS ${NECKLACE_MAP_TARGET}
    EXPORT ${GEOVIZ_EXPORT_TARGETS}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
)
install_files_and_directories("${HDRS}" ${INSTALL_NECKLACE_MAP_DIR})
install_files_and_directories("${SRCS}" ${INSTALL_NECKLACE_MAP_DIR})
