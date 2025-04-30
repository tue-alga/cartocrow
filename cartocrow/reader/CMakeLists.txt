set(SOURCES
	ipe_reader.cpp
)
set(HEADERS
	ipe_reader.h
)

add_library(reader ${SOURCES})
target_link_libraries(reader
	PUBLIC core
		   renderer
)

cartocrow_install_module(reader)
install(FILES ${HEADERS} DESTINATION ${CARTOCROW_INSTALL_DIR}/reader)
