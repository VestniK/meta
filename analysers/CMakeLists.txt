set(SRC
  actions.cpp
  metaprocessor.cpp
  reachabilitychecker.cpp
  resolver.cpp
  typechecker.cpp
)

add_library(analysers STATIC ${SRC})
target_link_libraries(analysers typesystem utils)

add_subdirectory(tests)
