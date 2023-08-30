set(SOURCES
	ellipse.cpp
	graph.cpp
	mosaic_cartogram.cpp
	orderly_spanning_tree.cpp
	painting.cpp
	triangulation.cpp
	visibility_drawing.cpp
)
set(HEADERS
	ellipse.h
	graph.h
	mosaic_cartogram.h
	orderly_spanning_tree.h
	painting.h
	parameters.h
	triangulation.h
	visibility_drawing.h
)

add_library(mosaic_cartogram ${SOURCES})
target_link_libraries(mosaic_cartogram
	PRIVATE Eigen3::Eigen
	PRIVATE glog::glog
)

install(TARGETS mosaic_cartogram
	EXPORT ${CARTOCROW_EXPORT_TARGETS}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
	INCLUDES DESTINATION ${CMAKE_INSTALL_SOURCE_DIR}
)
