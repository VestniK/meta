include(TestTools)

AddGTest(CppGenTests
  cppwriter.cpp
)
target_link_libraries(CppGenTests parser typesystem)
