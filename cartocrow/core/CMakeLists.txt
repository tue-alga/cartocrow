cartocrow_add_library(core ${CARTOCROW_INSTALL_DIR}/core)
target_link_libraries(core
	PUBLIC Ipe::ipelib
	PRIVATE glog::glog
)
