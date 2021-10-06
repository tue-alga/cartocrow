# The necklace_map library implements the Necklace Map geo-visualization method.
set(NECKLACE_MAP_TARGET necklace_map)

cartocrow_add_library(${NECKLACE_MAP_TARGET} ${CARTOCROW_INSTALL_DIR}/${NECKLACE_MAP_TARGET})
target_link_libraries(${NECKLACE_MAP_TARGET} Ipe::Ipelib)

set(NECKLACE_MAP_TARGET ${NECKLACE_MAP_TARGET} PARENT_SCOPE)

