# The CartoCrow common library implements functionality shared by different CartoCrow methods.
set(CARTOCROW_COMMON_TARGET common)

cartocrow_add_library(${CARTOCROW_COMMON_TARGET} ${CARTOCROW_INSTALL_DIR}/${CARTOCROW_COMMON_TARGET})

set(CARTOCROW_COMMON_TARGET ${CARTOCROW_COMMON_TARGET} PARENT_SCOPE)
