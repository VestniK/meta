#ifndef LLVMGENVISITOR_H
#define LLVMGENVISITOR_H

#include <map>
#include <stack>
#include <string>

#include <llvm/IR/IRBuilder.h>

#include "parser/metaparser.h"
#include "parser/metanodes.h"

#include "analysers/evaluator.h"

#include "generators/llvmgen/environment.h"
#include "generators/llvmgen/privateheadercheck.h"

namespace generators {
namespace llvmgen {

class ModuleBuilder: public analysers::Evaluator<llvm::Value*>
{
public:
    ModuleBuilder(Environment &env): env(env), builder(env.context) {}

    // Value consumers
    virtual void returnValue(meta::Return *node, llvm::Value *val) override;
    virtual void returnVoid (meta::Return *node) override;
    virtual void varInit(meta::VarDecl *node, llvm::Value *val) override;
    virtual void ifCond (meta::If *node, llvm::Value *val) override;
    // Value providers
    virtual llvm::Value *number(meta::Number *node) override;
    virtual llvm::Value *literal(meta::Literal *node) override;
    virtual llvm::Value *strLiteral(meta::StrLiteral *node) override;
    virtual llvm::Value *var(meta::Var *node) override;
    // Operations on values
    virtual llvm::Value *call(meta::Call *node, const std::vector<llvm::Value*> &args) override;
    virtual llvm::Value* assign(meta::Assigment *node, llvm::Value *val) override;
    virtual llvm::Value *binaryOp(meta::BinaryOp *node, llvm::Value *left, llvm::Value *right) override;
    virtual llvm::Value *prefixOp(meta::PrefixOp *node, llvm::Value *val);

    void save(const std::string &path);

    // Additional traverse functions
    virtual bool visit(meta::Function *node) override;
    // Do not generate code of func argumet default value calculation in the beggining of function since
    // it will be generated on call with default value
    virtual bool visit(meta::VarDecl *node) override {return !node->is(meta::VarDecl::argument) && analysers::Evaluator<llvm::Value*>::visit(node);}
    virtual void leave(meta::VarDecl *node) override {if (!node->is(meta::VarDecl::argument)) analysers::Evaluator<llvm::Value*>::leave(node);}

private:
    Environment &env;
    std::map<meta::VarDecl *, llvm::Value *> mVarMap;
    llvm::IRBuilder<> builder;
    bool mCurrBlockTerminated = false;
};

} // namespace llvmgen
} // namespace generators

#endif // LLVMGENVISITOR_H
