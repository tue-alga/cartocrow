set(SOURCES
    cmd_arguments.cpp
	persistent_settings.cpp
	stopwatch.cpp
)
set(HEADERS
    cmd_arguments.h
	persistent_settings.h
	stopwatch.h
)

add_library(utils ${SOURCES})

cartocrow_install_module(utils)
install(FILES ${HEADERS} DESTINATION ${CARTOCROW_INSTALL_DIR}/utils)
