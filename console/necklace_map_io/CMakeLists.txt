# The common_cla library collects functionality that is reused in several command-line applications.
set(NECKLACE_MAP_IO_TARGET "necklace_map_io")
set(NECKLACE_MAP_IO_TARGET ${NECKLACE_MAP_IO_TARGET} PARENT_SCOPE)

geoviz_add_console_library(${NECKLACE_MAP_IO_TARGET} ${INSTALL_CONSOLE_DIR}/${NECKLACE_MAP_IO_TARGET})
