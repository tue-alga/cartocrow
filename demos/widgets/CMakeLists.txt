set(SOURCES
    double_slider.h
)

add_library(widgets ${SOURCES})

target_link_libraries(
    widgets
    PRIVATE
    ${COMMON_CLA_TARGET}
    Qt5::Widgets
)

install(TARGETS widgets DESTINATION ${INSTALL_BINARY_DIR})
