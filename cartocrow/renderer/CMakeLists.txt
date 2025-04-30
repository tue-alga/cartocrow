set(SOURCES
	geometry_renderer.cpp
	geometry_widget.cpp
	ipe_renderer.cpp
	painting_renderer.cpp
	function_painting.cpp
	render_path.cpp
	svg_renderer.cpp
)
set(HEADERS
	geometry_painting.h
	geometry_renderer.h
	geometry_widget.h
	ipe_renderer.h
	painting_renderer.h
	function_painting.h
	render_path.h
	svg_renderer.h
)

add_library(renderer ${SOURCES})
target_link_libraries(renderer
	PUBLIC core Qt5::Widgets Ipe::ipelib
	PRIVATE glog::glog
)

if(WIN32)
	find_package(ZLIB REQUIRED)
	target_link_libraries(renderer
		PUBLIC gdiplus ${ZLIB_LIBRARIES}
	)
endif()

cartocrow_install_module(renderer)
install(FILES ${HEADERS} DESTINATION ${CARTOCROW_INSTALL_DIR}/renderer)
