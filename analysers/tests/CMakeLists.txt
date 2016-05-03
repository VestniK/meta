include(TestTools)

AddGTest(AnalysersTests
  metaprocessor.cpp
  reachability.cpp
  resolver.cpp
  typechecker.cpp
)
target_link_libraries(AnalysersTests analysers parser)
