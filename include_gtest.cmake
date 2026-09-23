set(GTEST_SOURCE "SYSTEM" CACHE STRING "Choose gtest source")


set(ALLOWED_GTEST_SOURCES "SYSTEM" "GITHUB")

set_property(CACHE GTEST_SOURCE PROPERTY STRINGS ALLOWED_GTEST_SOURCES)


if(NOT GTEST_SOURCE IN_LIST ALLOWED_GTEST_SOURCES)
    message(FATAL_ERROR "Invalid backend! Choose one of: ${ALLOWED_GTEST_SOURCES}")
endif()

message(STATUS "Selected gtest source is: ${GTEST_SOURCE}")


if (GTEST_SOURCE STREQUAL "SYSTEM")
  message(STATUS "Detecting GTest package")
  find_package(GTest CONFIG REQUIRED)
  message(STATUS "Detecting GTest package - done")
endif()

if (GTEST_SOURCE STREQUAL "GITHUB")
  message(STATUS "downloading GTest")
  include(FetchContent)
  FetchContent_Declare(
  googletest
  DOWNLOAD_EXTRACT_TIMESTAMP ON
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
  message(STATUS "GTest configured")
endif()

include(GoogleTest)
