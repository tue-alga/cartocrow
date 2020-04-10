# The test_flow_diagram executable exposes the Flow Diagram geo-visualization method unit tests.
geoviz_add_test(flow_diagram)

test_link_libraries(
    ${FLOW_DIAGRAM_TARGET}
    CGAL::CGAL
)
