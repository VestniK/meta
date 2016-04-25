find_package(GTest REQUIRED)
find_package(Threads REQUIRED) # Required by gtest but their *-config.cmake do not do it properly
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(ParserTests
  arythmetic.cpp
  lexer.cpp
  parser.cpp
  priority.cpp
)
target_link_libraries(ParserTests parser ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_test(NAME ParserTests
  COMMAND ParserTests --gtest_output=xml:${CMAKE_BINARY_DIR}/Reports/ParserTests.xml
)
