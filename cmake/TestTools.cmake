find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

macro(AddGTest TstName)
  add_executable(${TstName} ${ARGN})
  target_link_libraries(${TstName} ${GTEST_BOTH_LIBRARIES} Threads::Threads)
  add_test(NAME ${TstName}
    COMMAND ${TstName} --gtest_output=xml:${CMAKE_BINARY_DIR}/Reports/${TstName}.xml
  )
endmacro()
