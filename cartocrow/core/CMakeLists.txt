set(SOURCES
	core.cpp
	ipe_reader.cpp
	polygon.cpp
	region_map.cpp
	timer.cpp
)
set(HEADERS
	core.h
	ipe_reader.h
	polygon.h
	region_map.h
	timer.h
)

add_library(core ${SOURCES})
target_link_libraries(core
	PUBLIC Ipe::ipelib
	PRIVATE glog::glog
)

install(TARGETS core
	EXPORT ${CARTOCROW_EXPORT_TARGETS}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
	INCLUDES DESTINATION ${CMAKE_INSTALL_SOURCE_DIR}
)

# install_files_and_directories("${HDRS}" ${CARTOCROW_INSTALL_DIR}/core)
