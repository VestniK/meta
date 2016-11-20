set(PUB_HDR
  actions.h
  declconflicts.h
  dictionary.h
  metaprocessor.h
  reachabilitychecker.h
  resolver.h
  semanticerror.h
  typechecker.h
)

set(IMP_HPP
  actions.hpp
  metaprocessor.hpp
  reachabilitychecker.hpp
  resolver.hpp
  typechecker.hpp
)

set(SRC
  lib.cpp
)

add_library(analysers STATIC ${SRC} ${IMP_HPP} ${PUB_HDR})
target_link_libraries(analysers typesystem utils)

add_subdirectory(tests)
