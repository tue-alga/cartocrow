set(SOURCES
	vw_simplification.cpp
)
set(HEADERS
	vw_simplification.h
)

add_library(vw_simplification ${SOURCES})
target_link_libraries(vw_simplification
	PUBLIC core
	PRIVATE glog::glog
)

install(
	TARGETS vw_simplification
	EXPORT ${CARTOCROW_EXPORT_TARGETS}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
	INCLUDES DESTINATION ${CMAKE_INSTALL_SOURCE_DIR}
)
