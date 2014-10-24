find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(AnalysersTests
  reachability.cpp
  resolver.cpp
  typechecker.cpp
)
target_link_libraries(AnalysersTests analysers parser fs ${GTEST_BOTH_LIBRARIES})
add_test(NAME AnalysersTests
  COMMAND AnalysersTests --gtest_output=xml:${CMAKE_BINARY_DIR}/Reports/AnalysersTests.xml
)