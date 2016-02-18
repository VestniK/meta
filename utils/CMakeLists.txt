set(SRC
  exception.cpp
)

add_library(utils STATIC ${SRC})
target_link_libraries(utils stdc++fs)
