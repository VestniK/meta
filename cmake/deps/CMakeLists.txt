# Download and build all dependencies in a single build environment
cmake_minimum_required(VERSION 3.0)

include(ExternalProject)

if (${CMAKE_VERSION} VERSION_GREATER "3.4.0")
  set(USES_TERMINAL_BUILD_ON USES_TERMINAL_BUILD On)
endif()

find_package(lrstar QUIET)
if (NOT lrstar_FOUND)
  ExternalProject_Add(lrstar
    PREFIX ${CMAKE_BINARY_DIR}
    GIT_REPOSITORY https://github.com/VestniK/lrstar.git
    GIT_TAG master
    CMAKE_CACHE_ARGS
      -DCMAKE_INSTALL_PREFIX:path=${CMAKE_BINARY_DIR}
    ${USES_TERMINAL_BUILD_ON}
  )
endif()
