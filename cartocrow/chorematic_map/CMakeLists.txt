set(SOURCES
	maximum_weight_disk.cpp
)
set(HEADERS
	maximum_weight_disk.h
)

add_library(chorematic_map ${SOURCES})
target_link_libraries(chorematic_map
	PUBLIC core
)

cartocrow_install_module(chorematic_map)
install(FILES ${HEADERS} DESTINATION ${CARTOCROW_INSTALL_DIR}/chorematic_map)
