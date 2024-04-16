set(SOURCES
	voronoi_helpers.cpp
	isoline_simplifier.cpp
	collapse.cpp
	collapse.h
	ipe_bezier_wrapper.cpp
	symmetric_difference.cpp
	symmetric_difference.h
)
set(HEADERS
	isoline.h
    types.h
    voronoi_helpers.h
	isoline_simplifier.h
	ipe_bezier_wrapper.h
)

add_library(isoline_simplification ${SOURCES})
target_link_libraries(isoline_simplification
	PUBLIC core
	PRIVATE glog::glog
)

cartocrow_install_module(isoline_simplification)
install(FILES ${HEADERS} DESTINATION ${CARTOCROW_INSTALL_DIR}/isoline_simplification)
