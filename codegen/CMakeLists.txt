find_package(LLVM REQUIRED)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

get_system_libs(LLVM_DEPS)
set(LLVM_DEPS ${LLVM_DEPS} curses dl)
set(LLVM_REQUIRED_LIBS LLVMBitWriter LLVMCore LLVMSupport)

set(SRC
  codegen.cpp
)

add_library(codegen STATIC ${SRC})
target_link_libraries(codegen ${LLVM_DEPS} ${LLVM_REQUIRED_LIBS} curses dl)