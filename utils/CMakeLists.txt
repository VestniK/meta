set(SRC
  exception.cpp
)

add_library(utils STATIC ${SRC})
target_link_libraries(utils stdc++fs)
target_compile_options(utils INTERFACE -std=c++1z -fconcepts)

add_subdirectory(tests)
