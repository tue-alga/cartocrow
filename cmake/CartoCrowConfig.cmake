include(CMakeFindDependencyMacro)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

find_dependency(CGAL REQUIRED)
find_path(CGAL_INCLUDE_DIR CGAL/Exact_predicates_inexact_constructions_kernel.h)
include_directories(${CGAL_INCLUDE_DIR})
link_libraries(${CGAL_LIBRARIES})
find_dependency(GMP REQUIRED)

find_dependency(Qt5Widgets REQUIRED)
find_dependency(Ipelib REQUIRED)
find_dependency(GDAL REQUIRED)

# Add the targets file
include("${CMAKE_CURRENT_LIST_DIR}/CartoCrowTargets.cmake")
