# Download and build all dependencies in a single build environment
cmake_minimum_required(VERSION 3.0)

include(ExternalProject)

if (${CMAKE_VERSION} VERSION_GREATER "3.4.0")
  set(USES_TERMINAL_BUILD_ON USES_TERMINAL_BUILD On)
endif()

ExternalProject_Add(gtest
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/deps
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG master # 'install' target and top level cmake are not released yet
  CMAKE_CACHE_ARGS
    -DBUILD_SHARED_LIBS:bool=Off
    -DCMAKE_INSTALL_PREFIX:path=${CMAKE_CURRENT_BINARY_DIR}/deps
  ${USES_TERMINAL_BUILD_ON}
)

ExternalProject_Add(llvm
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/deps
  SVN_REPOSITORY http://llvm.org/svn/llvm-project/llvm/tags/RELEASE_371/final/
  CMAKE_CACHE_ARGS
    -DCMAKE_INSTALL_PREFIX:path=${CMAKE_CURRENT_BINARY_DIR}/deps
  ${USES_TERMINAL_BUILD_ON}
)

ExternalProject_Add(lrstar
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/deps
  GIT_REPOSITORY https://github.com/VestniK/lrstar.git
  GIT_TAG master
  CMAKE_CACHE_ARGS
    -DCMAKE_INSTALL_PREFIX:path=${CMAKE_CURRENT_BINARY_DIR}/deps
  ${USES_TERMINAL_BUILD_ON}
)

ExternalProject_Add(Meta
  DEPENDS gtest llvm lrstar
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/meta
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..
  CMAKE_CACHE_ARGS -DCMAKE_PREFIX_PATH:path=${CMAKE_CURRENT_BINARY_DIR}/deps
  ${USES_TERMINAL_BUILD_ON}
)

