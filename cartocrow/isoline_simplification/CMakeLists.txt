set(SOURCES
	voronoi_helpers.cpp
	isoline_simplifier.cpp
	collapse.cpp
	ipe_bezier_wrapper.cpp
	symmetric_difference.cpp
	simple_isoline_painting.cpp
	ipe_isolines.cpp
	simple_smoothing.cpp
	voronoi_helpers_cgal.cpp
	voronoi_helpers_cgal.h
)
set(HEADERS
	isoline.h
    types.h
    voronoi_helpers.h
	isoline_simplifier.h
	ipe_bezier_wrapper.h
	collapse.h
	ipe_isolines.h
	simple_isoline_painting.h
	symmetric_difference.h
	simple_smoothing.h
)

add_library(isoline_simplification ${SOURCES})
target_link_libraries(isoline_simplification
	PUBLIC core
)

cartocrow_install_module(isoline_simplification)
install(FILES ${HEADERS} DESTINATION ${CARTOCROW_INSTALL_DIR}/isoline_simplification)
