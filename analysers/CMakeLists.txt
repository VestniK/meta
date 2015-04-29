set(SRC
  metaprocessor.cpp
  reachabilitychecker.cpp
  resolver.cpp
  semanticerror.cpp
  typechecker.cpp
)

add_library(analysers STATIC ${SRC})
target_link_libraries(analysers typesysytem utils)

add_subdirectory(tests)
