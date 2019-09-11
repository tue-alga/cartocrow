# The necklace_map_cla executable exposes the Necklace Map geo-visualization method.
set(TARGET_CLA_NECKLACE_MAP necklace_map_cla)

# - While CMake will try various file extensions when adding files,
#   specifying the files without extension may give problems due to ambiguity.
file(GLOB_RECURSE HDRS "*.h")
file(GLOB_RECURSE SRCS "*.cpp")

add_executable( ${TARGET_CLA_NECKLACE_MAP} ${HDRS} ${SRCS} )
add_exe_properties(${TARGET_CLA_NECKLACE_MAP})

#target_link_libraries(${TARGET_CLA_NECKLACE_MAP} ${TARGET_LIB_NECKLACE_MAP})
target_link_libraries(${TARGET_CLA_NECKLACE_MAP} ${TARGET_LIB_GEOVIZ})

# Install rules.
set(INSTALL_NECLACE_MAP_CLA_DIR ${INSTALL_CONSOLE_DIR}/necklace_map)
install(TARGETS ${TARGET_CLA_NECKLACE_MAP} DESTINATION ${CMAKE_INSTALL_BINARY_DIR})
install_files_and_directories("${HDRS}" ${INSTALL_NECLACE_MAP_CLA_DIR})
install_files_and_directories("${SRCS}" ${INSTALL_NECLACE_MAP_CLA_DIR})
