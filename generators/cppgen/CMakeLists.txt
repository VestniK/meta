add_definitions(-DCPPGEN_BUILD) # used for private headers check

set(SRC
  generator.cpp
)

add_library(cppgenerator STATIC ${SRC})
target_link_libraries(analysers)
