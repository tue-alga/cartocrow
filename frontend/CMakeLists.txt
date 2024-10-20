set(SOURCES
    cartocrow.cpp
)

add_executable(cartocrow ${SOURCES})

find_package(nlohmann_json REQUIRED)

target_link_libraries(
    cartocrow
    PRIVATE
    ${COMMON_CLA_TARGET}
    core
    flow_map
    necklace_map
    isoline_simplification
    renderer
    CGAL::CGAL
    glog::glog
    nlohmann_json::nlohmann_json
)

install(TARGETS cartocrow DESTINATION ${INSTALL_BINARY_DIR})
