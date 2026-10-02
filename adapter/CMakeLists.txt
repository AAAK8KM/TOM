# there we can add adapters that bring outside data into the library
#

find_package(CURL REQUIRED)

add_library(orbmech_iers STATIC IERSFetch.cpp)
target_include_directories(orbmech_iers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(orbmech_iers PRIVATE ${COMPILE_FLAGS})
target_link_libraries(orbmech_iers PUBLIC orbmech_lib PRIVATE CURL::libcurl)

add_executable(iers_tests tests/test_iers.cpp)
target_compile_options(iers_tests PRIVATE ${COMPILE_FLAGS})
target_compile_definitions(iers_tests
  PRIVATE ORBMECH_IERS_TEST_DATA="${CMAKE_CURRENT_SOURCE_DIR}/tests/data")
target_link_libraries(iers_tests PRIVATE orbmech_iers GTest::gtest_main)

gtest_discover_tests(iers_tests)
