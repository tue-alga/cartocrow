include(FindPackageHandleStandardArgs)

find_path(Ipelib_INCLUDE_DIR NAMES ipelib.h)
find_library(Ipelib_LIBRARY NAMES ipe)
find_package_handle_standard_args(Ipelib
	DEFAULT_MSG Ipelib_LIBRARY Ipelib_INCLUDE_DIR)

if(Ipelib_FOUND)
	add_library(Ipe::Ipelib UNKNOWN IMPORTED)
	set_target_properties(Ipe::Ipelib PROPERTIES
		IMPORTED_LOCATION "${Ipelib_LIBRARY}"
		INTERFACE_INCLUDE_DIRECTORIES "${Ipelib_INCLUDE_DIR}"
	)
endif()

mark_as_advanced(Ipelib_LIBRARY Ipelib_INCLUDE_DIR)

