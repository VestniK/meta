include(TestTools)

AddGTest(UtilsTests
  range.cpp
)
target_link_libraries(UtilsTests utils)
