find_package(lrstar)

lrstar_grammar(GRM_SRC meta SKEL c++11)
add_library(parser STATIC ${GRM_SRC})
