set(SRC
  reachabilitychecker.cpp
  resolver.cpp
  semanticerror.cpp
  typechecker.cpp
)

add_library(analysers STATIC ${SRC})
target_link_libraries(analysers typesysytem)

add_subdirectory(tests)