set(SOURCES
	vw_simplification.cpp
)
set(HEADERS
	vw_simplification.h
)

add_library(simplification ${SOURCES})
target_link_libraries(simplification
	PUBLIC core
)

cartocrow_install_module(simplification)
install(FILES ${HEADERS} DESTINATION ${CARTOCROW_INSTALL_DIR}/simplification)