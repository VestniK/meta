include(TestTools)

AddGTest(ParserTests
  arythmetic.cpp
  lexer.cpp
  parser.cpp
  priority.cpp
)
target_link_libraries(ParserTests parser)
