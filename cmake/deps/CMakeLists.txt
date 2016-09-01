# Download and build all dependencies in a single build environment
cmake_minimum_required(VERSION 3.0)

include(ExternalProject)

if (${CMAKE_VERSION} VERSION_GREATER "3.4.0")
  set(USES_TERMINAL_BUILD_ON USES_TERMINAL_BUILD On)
endif()

find_package(GTest QUIET)
if (NOT GTest_FOUND)
  ExternalProject_Add(GTest
    PREFIX ${CMAKE_BINARY_DIR}
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.8.0
    CMAKE_CACHE_ARGS
      -DBUILD_SHARED_LIBS:bool=Off
      -DCMAKE_INSTALL_PREFIX:path=${CMAKE_BINARY_DIR}
    ${USES_TERMINAL_BUILD_ON}
  )
endif()

find_package(LLVM QUIET)
if (NOT LLVM_FOUND)
  ExternalProject_Add(llvm
    PREFIX ${CMAKE_BINARY_DIR}
    SVN_REPOSITORY http://llvm.org/svn/llvm-project/llvm/tags/RELEASE_381/final/
    CMAKE_CACHE_ARGS
      -DLLVM_ENABLE_RTTI:bool=On
      -DCMAKE_INSTALL_PREFIX:path=${CMAKE_BINARY_DIR}
    ${USES_TERMINAL_BUILD_ON}
  )
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
