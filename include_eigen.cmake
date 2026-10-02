set(EIGEN_SOURCE "SYSTEM" CACHE STRING "Choose eigen source")


set(ALLOWED_EIGEN_SOURCES "SYSTEM" "GITLAB")

set_property(CACHE EIGEN_SOURCE PROPERTY STRINGS ALLOWED_EIGEN_SOURCES)


if(NOT EIGEN_SOURCE IN_LIST ALLOWED_EIGEN_SOURCES)
    message(FATAL_ERROR "Invalid backend! Choose one of: ${ALLOWED_EIGEN_SOURCES}")
endif()

message(STATUS "Selected eigen source is: ${EIGEN_SOURCE}")


if (EIGEN_SOURCE STREQUAL "SYSTEM")
  message(STATUS "Detecting Eigen3 package")
  find_package(Eigen3 CONFIG REQUIRED)
  message(STATUS "Detecting Eigen3 package - done")
endif()

if (EIGEN_SOURCE STREQUAL "GITLAB")
  message(STATUS "downloading Eigen")
  include(FetchContent)
  FetchContent_Declare(
  eigen
  DOWNLOAD_EXTRACT_TIMESTAMP ON
  URL https://gitlab.com/libeigen/eigen/-/archive/5.0.1/eigen-5.0.1.tar.gz)
  set(EIGEN_BUILD_DOC OFF CACHE BOOL "" FORCE)
  set(EIGEN_BUILD_TESTING OFF CACHE BOOL "" FORCE)
  set(EIGEN_BUILD_PKGCONFIG OFF CACHE BOOL "" FORCE)
  set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(eigen)
  message(STATUS "Eigen configured")
endif()
