include(TestTools)

set(IMP_HPP
  actions.hpp
  metaprocessor.hpp
  reachability.hpp
  resolver.hpp
  resolve_call.hpp
  resolve_imports.hpp
  resolve_vars.hpp
  typechecker.hpp
)

AddGTest(AnalysersTests
  tests.cpp
  ${IMP_HPP}
)
target_link_libraries(AnalysersTests analysers parser)
