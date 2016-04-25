find_package(GTest REQUIRED)
find_package(Threads REQUIRED) # Required by gtest but their *-config.cmake do not do it properly
include_directories(${GTEST_INCLUDE_DIRS})

find_program(LLC llc)

set(meta_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/int.meta
  ${CMAKE_CURRENT_SOURCE_DIR}/bool.meta
  ${CMAKE_CURRENT_SOURCE_DIR}/ccall.meta
  ${CMAKE_CURRENT_SOURCE_DIR}/imports.meta
  ${CMAKE_CURRENT_SOURCE_DIR}/importsImpl.meta
)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test.bc
  COMMAND meta ${meta_SRC} -o ${CMAKE_CURRENT_BINARY_DIR}/test.bc
  MAIN_DEPENDENCY ${meta_SRC}
  DEPENDS meta ${meta_SRC}
)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test.o
  COMMAND ${LLC} -filetype=obj -O=2 ${CMAKE_CURRENT_BINARY_DIR}/test.bc -o ${CMAKE_CURRENT_BINARY_DIR}/test.o
  MAIN_DEPENDENCY ${CMAKE_CURRENT_BINARY_DIR}/test.bc
)

add_executable(BuilderTests
  generate.cpp
  test.o
)

target_link_libraries(BuilderTests ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_test(NAME BuilderTests
  COMMAND BuilderTests --gtest_output=xml:${CMAKE_BINARY_DIR}/Reports/BuilderTests.xml
)
