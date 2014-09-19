find_package(lrstar)

# Needed to include metanodes.h from parser cpp file inside current binary dir
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SRC
  actions.cpp
  assigment.cpp
  binaryop.cpp
  call.cpp
  codeblock.cpp
  exprstatement.cpp
  function.cpp
  number.cpp
  prefixop.cpp
  return.cpp
  var.cpp
  vardecl.cpp
)

lrstar_grammar(GRM_SRC meta SKEL c++11)
add_library(parser STATIC ${GRM_SRC} ${SRC})

add_subdirectory(tests)
