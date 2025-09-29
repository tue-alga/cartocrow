set(SOURCES
	ipe_reader.cpp
	gdal_conversion.cpp
	boundary_map_reader.cpp
	region_map_reader.cpp
)
set(HEADERS
	ipe_reader.h
	gdal_conversion.h
	boundary_map_reader.h
	region_map_reader.h
)

add_library(reader ${SOURCES})
target_link_libraries(reader
	PUBLIC core
		   renderer
		   Ipe::ipelib
	PRIVATE GDAL::GDAL
)

cartocrow_install_module(reader)
install(FILES ${HEADERS} DESTINATION ${CARTOCROW_INSTALL_DIR}/reader)
