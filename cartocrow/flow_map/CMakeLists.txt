cartocrow_add_library(flow_map ${CARTOCROW_INSTALL_DIR}/flow_map)
target_link_libraries(flow_map
	PRIVATE glog::glog
)
