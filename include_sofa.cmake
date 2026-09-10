

set(SOFA_SOURCE "SYSTEM" CACHE STRING "Choose sofa source")


set(ALLOWED_SOFA_SOURCES "SYSTEM" "IAU")

set_property(CACHE SOFA_SOURCE PROPERTY STRINGS ALLOWED_SOFA_SOURCES)


if(NOT SOFA_SOURCE IN_LIST ALLOWED_SOFA_SOURCES)
    message(FATAL_ERROR "Invalid backend! Choose one of: ${ALLOWED_SOFA_SOURCES}")
endif()

message(STATUS "Selected sofa source is: ${SOFA_SOURCE}")


if (SOFA_SOURCE STREQUAL "SYSTEM")
  message(STATUS "Detecting SOFA package")
  find_package(SOFA CONFIG REQUIRED)
  message(STATUS "Detecting SOFA package - done")
endif()

if (SOFA_SOURCE STREQUAL "IAU")
  message(STATUS "downloading SOFA")
  include(FetchContent)
  FetchContent_Declare(
  sofa_d
  DOWNLOAD_EXTRACT_TIMESTAMP ON
  URL https://www.iausofa.org/s/sofa_c-20231011tar.Z)
  FetchContent_MakeAvailable(sofa_d)
  set(sofa_src ${sofa_d_SOURCE_DIR}/20231011/c/src)
  file(GLOB sofa_files ${sofa_src}/*.c)
  add_library(sofa_l STATIC ${sofa_files})
  set_target_properties(sofa_l PROPERTIES LINKER_LANGUAGE C)
  target_compile_options(sofa_l PRIVATE -Wall -O)
  target_include_directories(sofa_l PUBLIC ${sofa_src})
  add_library(SOFA::sofa ALIAS sofa_l)
  message(STATUS "SOFA configured")
endif()
