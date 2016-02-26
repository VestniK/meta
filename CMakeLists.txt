project(meta)
cmake_minimum_required(VERSION 3.0)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

option(BUILD_DEPS "Download and build dependencies as part of the build" Off)

if (BUILD_DEPS)
  include(BuildWithDependencies)
  return()
endif()

find_package(Boost COMPONENTS program_options REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
enable_testing()

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
add_definitions(-Wall -Werror -std=c++14)

add_subdirectory(utils)
add_subdirectory(parser)
add_subdirectory(typesystem)
add_subdirectory(analysers)
add_subdirectory(generators)

add_executable(meta main.cpp)
target_link_libraries(meta parser analysers llvmgenerator cppgenerator ${Boost_LIBRARIES})
install(TARGETS meta RUNTIME DESTINATION bin)
