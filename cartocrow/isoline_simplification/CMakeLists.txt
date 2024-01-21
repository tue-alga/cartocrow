set(SOURCES
	medial_axis_separator.cpp
	isoline_simplifier.cpp
)
set(HEADERS
	isoline.h
    types.h
	medial_axis_separator.h
	isoline_simplifier.h
)

add_library(isoline_simplification ${SOURCES})
target_link_libraries(isoline_simplification
	PUBLIC core
	PRIVATE glog::glog
)

cartocrow_install_module(isoline_simplification)
install(FILES ${HEADERS} DESTINATION ${CARTOCROW_INSTALL_DIR}/isoline_simplification)
