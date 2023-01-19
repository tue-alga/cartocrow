set(SOURCES
	mosaic_cartogram.cpp
	painting.cpp
	parameters.cpp
)
set(HEADERS
	mosaic_cartogram.h
	painting.h
	parameters.h
)

add_library(mosaic_cartogram ${SOURCES})
target_link_libraries(mosaic_cartogram
	PRIVATE glog::glog
)

install(TARGETS mosaic_cartogram
	EXPORT ${CARTOCROW_EXPORT_TARGETS}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
	INCLUDES DESTINATION ${CMAKE_INSTALL_SOURCE_DIR}
)
