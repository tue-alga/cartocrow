# The multi_library_app_cla executable demonstrates linking multiple libraries to one executable.
geoviz_add_cla(multi_library_app)

target_link_libraries(
    ${TARGET}
    PRIVATE
    ${GEOVIZ_TARGET}
    CGAL::CGAL
)
