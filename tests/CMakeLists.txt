# there we can add test executables for project
#

add_executable(unit_tests
  test_delta_handler.cpp
  test_time_arith.cpp
  test_time_convert.cpp)

target_compile_options(unit_tests PRIVATE ${COMPILE_FLAGS})
target_link_libraries(unit_tests PRIVATE orbmech_lib GTest::gtest_main)

gtest_discover_tests(unit_tests)

# Hand run scratch program: prints the resolved conversion paths and the time
# arithmetic, no assertions, so it is built but not registered with ctest.
add_executable(test_time test_time.cpp)
target_compile_options(test_time PRIVATE ${COMPILE_FLAGS})
target_link_libraries(test_time PRIVATE orbmech_lib)
