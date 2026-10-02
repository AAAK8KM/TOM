
# there we can add subdirs of lib for project
#

add_library(orbmech_lib INTERFACE)
target_include_directories(orbmech_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(orbmech_lib INTERFACE SOFA::sofa Eigen3::Eigen)
