find_package(lrstar)

# Needed to include metanodes.h from parser cpp file inside current binary dir
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SRC
  metanodes.cpp
)

lrstar_grammar(GRM_SRC meta SKEL c++11)
add_library(parser STATIC ${GRM_SRC} ${SRC})

add_subdirectory(tests)
