# there we can add test executables for project
#

add_executable(unit_tests
  test_delta_handler.cpp
  test_time_arith.cpp
  test_time_convert.cpp)

target_compile_options(unit_tests PRIVATE ${COMPILE_FLAGS})
target_link_libraries(unit_tests PRIVATE orbmech_lib GTest::gtest_main)

gtest_discover_tests(unit_tests)
