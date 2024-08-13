set(SOURCES
	pattern.cpp
	island.cpp
	bank.cpp
)
set(HEADERS
	pattern.h
	types.h
	settings.h
	cat_point.h
	island.h
	bank.h
	point_voronoi_helpers.h
	cropped_voronoi.h
)

add_library(simplesets ${SOURCES})
target_link_libraries(simplesets
	PUBLIC core
)

cartocrow_install_module(simplesets)
install(FILES ${HEADERS} DESTINATION ${CARTOCROW_INSTALL_DIR}/simplesets)
