cartocrow_add_library(renderer ${CARTOCROW_INSTALL_DIR}/renderer)
target_link_libraries(renderer Qt5::Widgets Ipe::Ipelib)
if(WIN32)
	target_link_libraries(renderer gdiplus)
	find_package(ZLIB REQUIRED)
	target_link_libraries(renderer ${ZLIB_LIBRARIES})
endif()

