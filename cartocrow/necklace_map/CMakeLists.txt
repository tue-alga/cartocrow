cartocrow_add_library(necklace_map ${CARTOCROW_INSTALL_DIR}/necklace_map)
target_link_libraries(necklace_map
	PUBLIC Ipe::ipelib
	PRIVATE glog::glog
)

