# The common_cla library collects functionality that is reused in several command-line applications.
set(COMMON_CLA_TARGET "common_cla")
set(COMMON_CLA_TARGET ${COMMON_CLA_TARGET} PARENT_SCOPE)

cartocrow_add_console_library(${COMMON_CLA_TARGET} "${INSTALL_CONSOLE_DIR}/common")
