#ifndef PRIVATE_HEADER_CHECK_H
#define PRIVATE_HEADER_CHECK_H
#    ifndef LLVMGEN_BUILD
#        error This header is private header of the LLVM generator and should not be used outside of the llvmgenerator library sources!
#    endif
#endif // PRIVATE_HEADER_CHECK_H
