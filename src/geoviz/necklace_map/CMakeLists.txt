# The necklace_map library implements the Necklace Map geo-visualization method.
set(TARGET_LIB_NECKLACE_MAP necklace_map)
set(TARGET_LIB_NECKLACE_MAP ${TARGET_LIB_NECKLACE_MAP} PARENT_SCOPE)

file(GLOB_RECURSE HDRS "*.h")
file(GLOB_RECURSE SRCS "*.cpp")

add_library(${TARGET_LIB_NECKLACE_MAP} ${HDRS} ${SRCS})

# Install rules.
set(INSTALL_NECKLACE_MAP_DIR ${INSTALL_GEOVIZ_DIR}/necklace_map)
install(
    TARGETS ${TARGET_LIB_NECKLACE_MAP}
    EXPORT ${INSTALL_GEOVIZ_EXPORT}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
)
install_files_and_directories("${HDRS}" ${INSTALL_NECKLACE_MAP_DIR})
install_files_and_directories("${SRCS}" ${INSTALL_NECKLACE_MAP_DIR})
