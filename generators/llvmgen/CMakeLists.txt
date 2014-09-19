find_package(LLVM REQUIRED)

add_definitions(${LLVM_DEFINITIONS})
add_definitions(-DLLVMGEN_BUILD) # used for private headers check
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

get_system_libs(LLVM_DEPS)
set(LLVM_DEPS ${LLVM_DEPS} curses dl)
set(LLVM_REQUIRED_LIBS LLVMBitWriter LLVMCore LLVMSupport)

set(SRC
  environment.cpp
  generator.cpp
  modulebuilder.cpp
)

add_library(llvmgenerator STATIC ${SRC})
target_link_libraries(llvmgenerator ${LLVM_DEPS} ${LLVM_REQUIRED_LIBS} curses dl abi)

add_subdirectory(tests)
