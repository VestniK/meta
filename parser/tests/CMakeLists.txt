include(TestTools)

AddGTest(ParserTests
  arythmetic.cpp
  lexer.cpp
  parse_functions.cpp
  parse_struct.cpp
  parser.cpp
  priority.cpp
)
target_link_libraries(ParserTests parser)
target_compile_definitions(ParserTests PRIVATE PARSER_ONLY_TEST)
