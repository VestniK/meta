project(meta)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
enable_testing()

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
add_definitions(-Wall -Werror -std=c++11)

add_subdirectory(parser)
add_subdirectory(fs)
add_subdirectory(builder)

add_executable(meta main.cpp)
target_link_libraries(meta parser fs builder)
install(TARGETS meta RUNTIME DESTINATION bin)
