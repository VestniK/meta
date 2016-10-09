find_package(lrstar)

set(SRC
  binaryop.cpp
  call.cpp
  codeblock.cpp
  exprstatement.cpp
  function.cpp
  if.cpp
  import.cpp
  literal.cpp
  number.cpp
  prefixop.cpp
  sourcefile.cpp
  strliteral.cpp
  struct.cpp
  var.cpp
  vardecl.cpp
  visibility.cpp
)

lrstar_grammar(GRM_SRC meta CUSTOM_SKEL ${CMAKE_CURRENT_SOURCE_DIR}/skels)
add_library(parser STATIC ${GRM_SRC} ${SRC})
target_link_libraries(parser utils)

add_subdirectory(tests)
