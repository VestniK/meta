project(meta)
cmake_minimum_required(VERSION 3.0)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(MissingDependencies)

find_package(Boost COMPONENTS program_options REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
enable_testing()

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
set(CMAKE_CXX_STANDARD 14)
add_definitions(-Wall -Werror)

add_subdirectory(utils)
add_subdirectory(parser)
add_subdirectory(typesystem)
add_subdirectory(analysers)
add_subdirectory(generators)

add_executable(meta main.cpp)
target_link_libraries(meta parser analysers llvmgenerator ${Boost_LIBRARIES})
install(TARGETS meta EXPORT meta
  RUNTIME DESTINATION bin
)

install(EXPORT meta NAMESPACE meta:: DESTINATION lib/cmake/meta)
file(WRITE ${CMAKE_BINARY_DIR}/meta-config.cmake
  "include(\"\${CMAKE_CURRENT_LIST_DIR}/meta.cmake\")"
)
install(FILES ${CMAKE_BINARY_DIR}/meta-config.cmake
  DESTINATION lib/cmake/meta
)

