#ifndef LLVMGENVISITOR_H
#define LLVMGENVISITOR_H

#include <map>
#include <stack>
#include <string>

#include <llvm/IR/IRBuilder.h>

#include "parser/metaparser.h"
#include "parser/metanodes.h"

#include "generators/translator.h"

#include "generators/llvmgen/environment.h"
#include "generators/llvmgen/privateheadercheck.h"

namespace generators {
namespace llvmgen {

class ModuleBuilder: public generators::Translator<llvm::Value*>
{
public:
    ModuleBuilder(Environment &env): env(env), builder(env.context) {}

    virtual void startFunction(meta::Function *node) override;
    // Value consumers
    virtual void declareVar(meta::VarDecl *node, llvm::Value *initialVal) override;
    virtual void returnValue(meta::Return *node, llvm::Value *val) override;
    // Value providers
    virtual llvm::Value *number(meta::Number *node) override;
    virtual llvm::Value *var(meta::Var *node) override;
    // Operations on values
    virtual llvm::Value *call(meta::Call *node, const std::vector<llvm::Value*> &args) override;
    virtual llvm::Value *assign(meta::Assigment *node, llvm::Value *val) override;
    virtual llvm::Value *binaryOp(meta::BinaryOp *node, llvm::Value *left, llvm::Value *right) override;

    void save(const std::string &path);

private:
    Environment &env;
    std::map<std::string, llvm::Value*> regVarMap; // IR registry allocated
    std::map<std::string, llvm::AllocaInst*> stackVarMap; // Stack allocated
    llvm::IRBuilder<> builder;
};

} // namespace llvmgen
} // namespace generators

#endif // LLVMGENVISITOR_H
