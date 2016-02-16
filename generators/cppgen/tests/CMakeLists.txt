find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(CppGenTests
  cppwriter.cpp
)

target_link_libraries(CppGenTests ${GTEST_BOTH_LIBRARIES} parser typesystem)
add_test(NAME CppGenTests
  COMMAND CppGenTests --gtest_output=xml:${CMAKE_BINARY_DIR}/Reports/CppGenTests.xml
)
