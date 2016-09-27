include(TestTools)

AddGTest(AnalysersTests
  actions.cpp
  metaprocessor.cpp
  reachability.cpp
  resolver.cpp
  resolve_call.cpp
  resolve_imports.cpp
  resolve_vars.cpp
  typechecker.cpp
)
target_link_libraries(AnalysersTests analysers parser)
