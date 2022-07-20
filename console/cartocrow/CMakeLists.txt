set(SOURCES
    cartocrow.cpp
)

add_executable(cartocrow ${SOURCES})

target_link_libraries(
    cartocrow
    PRIVATE
    ${COMMON_CLA_TARGET}
    necklace_map
    core
    renderer
    CGAL::CGAL
    Qt5::Widgets
    glog::glog
    nlohmann_json::nlohmann_json
)

install(TARGETS cartocrow DESTINATION ${INSTALL_BINARY_DIR})
