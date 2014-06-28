#ifndef CODEGEN_H
#define CODEGEN_H

#include <string>
#include <memory>

#include "parser/metaparser.h"

namespace llvm {

class LLVMContext;
class Module;

} // namespace llvm

struct BuildContext;

class CodeGen: public meta::Visitor
{
public:
    CodeGen(const std::string &package);
    virtual ~CodeGen();

    virtual void visit(meta::Function *node) override;

    virtual void visit(meta::Call *) override;

    virtual void visit(meta::Number *node) override;
    virtual void visit(meta::Var *node) override;

    virtual void leave(meta::BinaryOp *node) override;
    virtual void leave(meta::Return *node) override;

    void save(const std::string &path);

private:
    llvm::LLVMContext &globalContext;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<BuildContext> buildContext;
};

#endif // CODEGEN_H
