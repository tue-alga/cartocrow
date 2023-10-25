set(SOURCES
	core.cpp
	ellipse.cpp
	ipe_reader.cpp
	region_arrangement.cpp
	region_map.cpp
	timer.cpp
)
set(HEADERS
	centroid.h
	core.h
	ellipse.h
	ipe_reader.h
	region_arrangement.h
	region_map.h
	timer.h
)

add_library(core ${SOURCES})
target_link_libraries(core
	PUBLIC Eigen3::Eigen Ipe::ipelib
	PRIVATE glog::glog
)

cartocrow_install_module(core)
install(FILES ${HEADERS} DESTINATION ${CARTOCROW_INSTALL_DIR}/core)
