include(FindPackageHandleStandardArgs)

find_path(Ipelib_INCLUDE_DIR NAMES ipelib.h)
find_library(Ipelib_LIBRARY NAMES ipe)
find_package_handle_standard_args(Ipelib
	DEFAULT_MSG Ipelib_LIBRARY Ipelib_INCLUDE_DIR)

if(Ipelib_FOUND)
	find_package(ZLIB REQUIRED)
	find_package(JPEG REQUIRED)
	find_package(PNG REQUIRED)

	add_library(Ipe::ipelib UNKNOWN IMPORTED)
	set_target_properties(Ipe::ipelib PROPERTIES
		IMPORTED_LOCATION "${Ipelib_LIBRARY}"
		INTERFACE_INCLUDE_DIRECTORIES "${Ipelib_INCLUDE_DIR}"
	)
	target_link_libraries(Ipe::ipelib INTERFACE JPEG::JPEG)
	target_link_libraries(Ipe::ipelib INTERFACE ZLIB::ZLIB)
	target_link_libraries(Ipe::ipelib INTERFACE PNG::PNG)
	if(WIN32)
		target_link_libraries(Ipe::ipelib INTERFACE gdiplus)
	endif()
endif()

mark_as_advanced(Ipelib_LIBRARY Ipelib_INCLUDE_DIR)
