find_package(GTest REQUIRED)
find_package(Threads REQUIRED) # Required by gtest but their *-config.cmake do not do it properly
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(CppGenTests
  cppwriter.cpp
)

target_link_libraries(CppGenTests ${GTEST_BOTH_LIBRARIES} parser typesystem ${CMAKE_THREAD_LIBS_INIT})
add_test(NAME CppGenTests
  COMMAND CppGenTests --gtest_output=xml:${CMAKE_BINARY_DIR}/Reports/CppGenTests.xml
)
