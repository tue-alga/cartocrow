cartocrow_add_library(vw_simplification ${CARTOCROW_INSTALL_DIR}/vw_simplification)
target_link_libraries(vw_simplification
	PRIVATE glog::glog
)
