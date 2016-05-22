include(TestTools)

AddGTest(ParserTests
  arythmetic.cpp
  lexer.cpp
  parse_functions.cpp
  parser.cpp
  priority.cpp
)
target_link_libraries(ParserTests parser)
