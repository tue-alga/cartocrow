# The GeoViz common library implements functionality shared by different GeoViz methods.
set(GEOVIZ_COMMON_TARGET common)

geoviz_add_library(${GEOVIZ_COMMON_TARGET} ${GEOVIZ_INSTALL_DIR}/${GEOVIZ_COMMON_TARGET})

set(GEOVIZ_COMMON_TARGET ${GEOVIZ_COMMON_TARGET} PARENT_SCOPE)
