set(SOURCES
	graph.cpp
	guiding_shape.cpp
	mosaic_cartogram.cpp
	orderly_spanning_tree.cpp
	painting.cpp
	tile_map.cpp
	triangulation.cpp
	visibility_drawing.cpp
)
set(HEADERS
	edmonds_optimum_branching_impl.hpp
	edmonds_optimum_branching.hpp
	graph.h
	guiding_shape.h
	mosaic_cartogram.h
	orderly_spanning_tree.h
	painting.h
	parameters.h
	region.h
	tile_map.h
	triangulation.h
	visibility_drawing.h
)

add_library(mosaic_cartogram ${SOURCES})
target_link_libraries(mosaic_cartogram
	PUBLIC core
	PRIVATE glog::glog
)

cartocrow_install_module(mosaic_cartogram)
install(FILES ${HEADERS} DESTINATION ${CARTOCROW_INSTALL_DIR}/mosaic_cartogram)
