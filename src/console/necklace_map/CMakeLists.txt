# The necklace_map_cla executable exposes the Necklace Map geo-visualization method.
set(NECKLACE_MAP_CLA_TARGET "${NECKLACE_MAP_TARGET}_cla")
set(NECKLACE_MAP_CLA_PHP "run_${NECKLACE_MAP_TARGET}.php")

# - While CMake will try various file extensions when adding files,
#   specifying the files without extension may give problems due to ambiguity.
file(GLOB_RECURSE HDRS "*.h")
file(GLOB_RECURSE SRCS "*.cpp")

add_executable( ${NECKLACE_MAP_CLA_TARGET} ${HDRS} ${SRCS} )
add_exe_properties(${NECKLACE_MAP_CLA_TARGET})

#target_link_libraries(${NECKLACE_MAP_CLA_TARGET} ${NECKLACE_MAP_TARGET})
target_link_libraries(
    ${NECKLACE_MAP_CLA_TARGET}
    ${GEOVIZ_TARGET}
    #${NECKLACE_MAP_TARGET}
    gflags
    glog
)

# Install rules.
set(INSTALL_NECKLACE_MAP_CLA_DIR ${INSTALL_CONSOLE_DIR}/necklace_map)
install(TARGETS ${NECKLACE_MAP_CLA_TARGET} DESTINATION ${CMAKE_INSTALL_BINARY_DIR})
install_files_and_directories("${HDRS}" ${INSTALL_NECKLACE_MAP_CLA_DIR})
install_files_and_directories("${SRCS}" ${INSTALL_NECKLACE_MAP_CLA_DIR})

# PHP script to run the executable.
# Note that the configure file uses the ${CURRENT_CLA} variable to determine the executable to run.
set(CURRENT_CLA ${NECKLACE_MAP_CLA_TARGET})
configure_file(
    "${SOURCE_CONFIG_DIR}/run_cla.php.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${NECKLACE_MAP_CLA_PHP}"
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${NECKLACE_MAP_CLA_PHP}" DESTINATION "${CMAKE_INSTALL_SCRIPT_DIR}")
