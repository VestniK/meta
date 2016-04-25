find_package(LLVM REQUIRED)

add_definitions(${LLVM_DEFINITIONS})
add_definitions(-DLLVMGEN_BUILD) # used for private headers check
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

set(LLVM_DEPS ${LLVM_DEPS} dl)
set(LLVM_REQUIRED_LIBS LLVMBitWriter LLVMCore LLVMSupport)

set(SRC
  environment.cpp
  expressionbuilder.cpp
  generator.cpp
  modulebuilder.cpp
)

add_library(llvmgenerator STATIC ${SRC})
target_link_libraries(llvmgenerator analysers ${LLVM_DEPS} ${LLVM_REQUIRED_LIBS} abi)

add_subdirectory(tests)
