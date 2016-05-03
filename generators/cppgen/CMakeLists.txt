find_package(ClangLibs REQUIRED)
add_definitions(-DCPPGEN_BUILD) # used for private headers check

set(SRC
  generator.cpp
)

add_library(cppgenerator STATIC ${SRC})
target_link_libraries(cppgenerator analysers clang::AST clang::Format)

add_subdirectory(tests)
