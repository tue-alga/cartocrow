set(SOURCES
	maximum_weight_disk.cpp
    sampler.h
	parse_points.cpp
	disk_area.cpp
	choropleth.cpp
	natural_breaks_external.cpp
	choropleth_disks.cpp
)
set(HEADERS
	maximum_weight_disk.h
	parse_points.h
	disk_area.h
	choropleth.h
	natural_breaks.h
	weighted_region_sample.h
	choropleth_disks.h
)

add_library(chorematic_map ${SOURCES})
target_link_libraries(chorematic_map
	PUBLIC core
	PRIVATE GDAL::GDAL
)

cartocrow_install_module(chorematic_map)
install(FILES ${HEADERS} DESTINATION ${CARTOCROW_INSTALL_DIR}/chorematic_map)
