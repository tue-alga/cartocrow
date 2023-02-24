set(SOURCES	
	vertex_removal/visvalingam_whyatt.cpp
)
set(HEADERS
	vertex_removal/visvalingam_whyatt.h
	vertex_removal/vertex_removal.h
	painting.h
	util.h
	modifiable_arrangement.h
	historic_arrangement.h
	oblivious_arrangement.h
)

add_library(simplification ${SOURCES})
target_link_libraries(simplification
	PUBLIC core
	PRIVATE glog::glog
)

install(
	TARGETS simplification
	EXPORT ${CARTOCROW_EXPORT_TARGETS}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
	INCLUDES DESTINATION ${CMAKE_INSTALL_SOURCE_DIR}
)
