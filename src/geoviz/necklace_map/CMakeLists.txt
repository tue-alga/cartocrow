# The necklace_map library implements the Necklace Map geo-visualization method.
set(TARGET_LIB_NECKLACE_MAP necklace_map)
set(TARGET_LIB_NECKLACE_MAP ${TARGET_LIB_NECKLACE_MAP} PARENT_SCOPE)

file(GLOB_RECURSE HDRS "*.h")
file(GLOB_RECURSE SRCS "*.cpp")

add_library(${TARGET_LIB_NECKLACE_MAP} ${HDRS} ${SRCS})

# Install rules.
install(TARGETS ${TARGET_LIB_NECKLACE_MAP} DESTINATION ${CMAKE_INSTALL_BINARY_DIR})
#install(DIRECTORY . DESTINATION ${INSTALL_GEOVIZ_DIR}/necklace_map)
install(FILES ${HDRS} ${SRCS} DESTINATION ${INSTALL_GEOVIZ_DIR}/necklace_map)
