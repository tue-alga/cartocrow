set(SOURCES
    edge_collapse/kronenfeld_etal.hpp
)
set(HEADERS
	vertex_removal/vertex_removal.h
	vertex_removal/visvalingam_whyatt.h
	edge_collapse/edge_collapse.h
	edge_collapse/kronenfeld_etal.h
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

cartocrow_install_module(simplification)
install(FILES ${HEADERS} DESTINATION ${CARTOCROW_INSTALL_DIR}/simplification)
