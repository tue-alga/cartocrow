set(SOURCES
	geometry_renderer.cpp
	geometry_widget.cpp
	ipe_renderer.cpp
)
set(HEADERS
	geometry_painting.h
	geometry_renderer.h
	geometry_widget.h
	ipe_renderer.h
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

install(
	TARGETS renderer
	EXPORT ${CARTOCROW_EXPORT_TARGETS}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
	INCLUDES DESTINATION ${CMAKE_INSTALL_SOURCE_DIR}
)

# install_files_and_directories(${HEADERS} ${CARTOCROW_INSTALL_DIR}/renderer)
