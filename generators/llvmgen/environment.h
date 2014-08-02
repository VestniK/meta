#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <memory>

namespace llvm {

class LLVMContext;
class Module;

}

namespace meta {

class Function;

}

namespace generators {
namespace llvmgen {

struct Environment
{
    Environment(const std::string &moduleName);
    ~Environment();

    void addFunction(meta::Function *func);

    llvm::LLVMContext &context;
    std::unique_ptr<llvm::Module> module;
};

} // namespace llvmgen
} // namespace generators

#endif // ENVIRONMENT_H

