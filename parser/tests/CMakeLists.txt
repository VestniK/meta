find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(ParserTests
  arythmetic.cpp
  lexer.cpp
  module.cpp
  parser.cpp
  priority.cpp
)
target_link_libraries(ParserTests parser fs ${GTEST_BOTH_LIBRARIES})
add_test(NAME ParserTests
  COMMAND ParserTests --gtest_output=xml:${CMAKE_BINARY_DIR}/Reports/ParserTests.xml
)
