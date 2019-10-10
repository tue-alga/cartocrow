# The common_cla library collects functionality that is reused in several command-line applications.
set(COMMON_CLA_TARGET "common_cla")
set(COMMON_CLA_TARGET ${COMMON_CLA_TARGET} PARENT_SCOPE)

file(GLOB_RECURSE HDRS "*.h")
file(GLOB_RECURSE SRCS "*.cpp")

add_library(${COMMON_CLA_TARGET} ${HDRS} ${SRCS})

# Install rules.
set(INSTALL_COMMON_CLA_DIR ${INSTALL_CONSOLE_DIR}/common)
install(
    TARGETS ${COMMON_CLA_TARGET}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
)
install_files_and_directories("${HDRS}" ${INSTALL_COMMON_CLA_DIR})
install_files_and_directories("${SRCS}" ${INSTALL_COMMON_CLA_DIR})