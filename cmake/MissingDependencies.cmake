option(BUILD_MISSING_DEPS "Download and build missing dependencies" On)

if (BUILD_MISSING_DEPS)
  message(STATUS "Dependencies build START")
  file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/deps)
  execute_process(
    COMMAND ${CMAKE_COMMAND} -G ${CMAKE_GENERATOR} ${CMAKE_SOURCE_DIR}/cmake/deps
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/deps
    RESULT_VARIABLE RETCODE
  )
  if (NOT ${RETCODE} EQUAL 0)
    message(FATAL_ERROR "Fetch and build dependecies failure")
  endif()
  execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/deps
    RESULT_VARIABLE RETCODE
  )
  if (NOT ${RETCODE} EQUAL 0)
    message(FATAL_ERROR "Fetch and build dependecies failure")
  endif()
  list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR}/deps)

  include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
  conan_basic_setup()

  message(STATUS "Dependencies build DONE")
endif()
