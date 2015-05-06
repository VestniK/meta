set(SRC
  metaprocessor.cpp
  nodeexception.cpp
  reachabilitychecker.cpp
  resolver.cpp
  semanticerror.cpp
  typechecker.cpp
  unexpectednode.cpp
)

add_library(analysers STATIC ${SRC})
target_link_libraries(analysers typesysytem utils)

add_subdirectory(tests)
