find_package(lrstar)

set(PUB_HDR
  annotation.h
  assigment.h
  binaryop.h
  call.h
  codeblock.h
  declaration.h
  expression.h
  exprstatement.h
  function.h
  if.h
  import.h
  literal.h
  memberaccess.h
  metanodes.h
  nodeexception.h
  number.h
  prefixop.h
  return.h
  sourcefile.h
  strliteral.h
  struct.h
  typed.h
  unexpectednode.h
  vardecl.h
  var.h
  visibility.h
)

set(IMP_HPP
  binaryop.hpp
  call.hpp
  codeblock.hpp
  exprstatement.hpp
  function.hpp
  if.hpp
  import.hpp
  literal.hpp
  number.hpp
  prefixop.hpp
  sourcefile.hpp
  strliteral.hpp
  struct.hpp
  var.hpp
  vardecl.hpp
  visibility.hpp
)

set(SRC lib.cpp)

lrstar_grammar(GRM_SRC meta CUSTOM_SKEL ${CMAKE_CURRENT_SOURCE_DIR}/skels)
add_library(parser STATIC ${GRM_SRC} ${SRC} ${IMP_HPP} ${PUB_HDR})
target_link_libraries(parser utils)

add_subdirectory(tests)
