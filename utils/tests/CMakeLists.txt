include(TestTools)

AddGTest(UtilsTests
  range.cpp
  string.cpp
)
target_link_libraries(UtilsTests utils)
