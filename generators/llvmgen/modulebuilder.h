#ifndef LLVMGENVISITOR_H
#define LLVMGENVISITOR_H

#include <map>
#include <stack>
#include <string>

#include <llvm/IR/IRBuilder.h>

#include "parser/metaparser.h"
#include "parser/metanodes.h"

#include "generators/llvmgen/environment.h"
#include "generators/llvmgen/privateheadercheck.h"

namespace generators {
namespace llvmgen {

class ModuleBuilder: public meta::Visitor
{
public:
    ModuleBuilder(Environment &env);
    virtual ~ModuleBuilder();

    virtual void visit(meta::Function *node) override;

    virtual void visit(meta::Call *) override;
    virtual void leave(meta::Call *node) override;

    virtual void visit(meta::Number *node) override;
    virtual void visit(meta::Var *node) override;
    virtual void leave(meta::VarDecl *node) override;
    virtual void leave(meta::Assigment *node) override;

    virtual void leave(meta::BinaryOp *node) override;
    virtual void leave(meta::Return *node) override;
    virtual void leave (meta::ExprStatement *);

    void save(const std::string &path);

private:
    Environment &env;
    std::map<std::string, llvm::Value*> regVarMap; // IR registry allocated
    std::map<std::string, llvm::AllocaInst*> stackVarMap; // Stack allocated
    std::stack<llvm::Value*> evaluationStack;
    llvm::IRBuilder<> builder;
};

} // namespace llvmgen
} // namespace generators

#endif // LLVMGENVISITOR_H
