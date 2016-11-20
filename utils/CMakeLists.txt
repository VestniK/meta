set(PUB_HDR
  array_view.h
  bitmask.h
  contract.h
  exception.h
  io.h
  property.h
  range.h
  sourcefile.h
  string.h
  term.h
  testtools.h
  types.h
)

set(IMP_HPP
  exception.hpp
  term.hpp
)

set(SRC
  lib.cpp
)

add_library(utils STATIC ${SRC} ${PUB_HDR} ${IMP_HPP})
target_link_libraries(utils stdc++fs)
target_compile_options(utils INTERFACE -std=c++1z -fconcepts)

add_subdirectory(tests)
