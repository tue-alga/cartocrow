set(SHARED_SOURCES
	function_painting.cpp
	geometry_renderer.cpp
	painting_renderer.cpp
	render_path.cpp
	svg_renderer.cpp
	)
set(NATIVE_SOURCES
	geometry_widget.cpp
	ipe_renderer.cpp
)
set(SHARED_HEADERS
	function_painting.h
	geometry_painting.h
	geometry_renderer.h
	graph_painting.h
	painting_renderer.h
	render_path.h
	svg_renderer.h
	voronoi_drawer.h
)
set(NATIVE_HEADERS
	ipe_renderer.h
	geometry_widget.h
)

if (EMSCRIPTEN)
set(SOURCES
    ${SHARED_SOURCES}
)
set(HEADERS
    ${SHARED_HEADERS}
)
else()
set(SOURCES
    ${SHARED_SOURCES}
    ${NATIVE_SOURCES}
)
set(HEADERS
    ${SHARED_HEADERS}
    ${NATIVE_HEADERS}
)
endif()

add_library(renderer ${SOURCES})
if(EMSCRIPTEN)
target_link_libraries(renderer
	PUBLIC core
)
else()
target_link_libraries(renderer
	PUBLIC core Qt5::Widgets Ipe::ipelib
)
endif()

cartocrow_install_module(renderer)
install(FILES ${HEADERS} DESTINATION ${CARTOCROW_INSTALL_DIR}/renderer)
