option(BUILD_MISSING_DEPS "Download and build missing dependencies" On)

if (BUILD_MISSING_DEPS)
  include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
  conan_basic_setup()
endif()
