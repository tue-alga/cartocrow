set(SOURCES
	
)
set(HEADERS
	indexed_priority_queue.h
	indexed_priority_queue.hpp
	point_quad_tree.h
	point_quad_tree.hpp
	quad_tree.h
	quad_tree.hpp
	graph_2.h
	graph_traits_2.h
	graph_map_2.h
	graph_operation_2.h
	graph_curve_traits_2.h
	graph_segment_traits_2.h
	straight_graph_2.h
)

add_library(data_structures ${SOURCES})

cartocrow_install_module(data_structures)
install(FILES ${HEADERS} DESTINATION ${CARTOCROW_INSTALL_DIR}/data_structures)
