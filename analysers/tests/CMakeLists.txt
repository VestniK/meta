include(TestTools)

AddGTest(AnalysersTests
  actions.cpp
  metaprocessor.cpp
  reachability.cpp
  resolver.cpp
  resolve_imports.cpp
  typechecker.cpp
)
target_link_libraries(AnalysersTests analysers parser)
