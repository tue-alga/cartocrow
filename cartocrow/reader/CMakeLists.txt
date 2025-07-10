set(SOURCES
	ipe_reader.cpp
	gdal_conversion.cpp
)
set(HEADERS
	ipe_reader.h
	gdal_conversion.h
)

add_library(reader ${SOURCES})
target_link_libraries(reader
	PUBLIC core
		   renderer
	PRIVATE GDAL::GDAL
)

cartocrow_install_module(reader)
install(FILES ${HEADERS} DESTINATION ${CARTOCROW_INSTALL_DIR}/reader)
