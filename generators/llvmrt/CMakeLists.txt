set(SRC
  arrays.c
)

add_library(meta-rt ${SRC})
set_target_properties(meta-rt PROPERTIES
  C_STANDARD 11
)

install(TARGETS meta-rt EXPORT meta
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
