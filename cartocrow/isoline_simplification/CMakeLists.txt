set(SOURCES
)
set(HEADERS
	isoline.h
)

add_library(isoline_simplification ${SOURCES})
target_link_libraries(isoline_simplification
	PUBLIC core
	PRIVATE glog::glog
)

cartocrow_install_module(isoline_simplification)
install(FILES ${HEADERS} DESTINATION ${CARTOCROW_INSTALL_DIR}/isoline_simplification)
