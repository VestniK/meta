#include <cassert>
#include <map>
#include <stack>
#include <stdexcept>
#include <vector>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/raw_ostream.h>

#include "parser/metanodes.h"

#include "codegen/codegen.h"

struct BuildContext
{
    BuildContext(): currFunc(nullptr), builder(llvm::getGlobalContext()) {}

    llvm::Function *currFunc;
    std::map<std::string, llvm::Value*> varMap;

    llvm::IRBuilder<> builder;
    std::stack<llvm::Value*> evaluationStack;
};

template<class T>
class ChildrenGatherer: public meta::Visitor
{
public:
    virtual void visit(T *node) override {mGathered.push_back(node);}

    const std::vector<T *> &gathered() const {return mGathered;}

private:
    std::vector<T *> mGathered;
};

CodeGen::CodeGen(const std::string &package):
    globalContext(llvm::getGlobalContext()), /// @todo move to BuildContext struct
    module(new llvm::Module(package, globalContext)),
    buildContext(new BuildContext)
{
}

CodeGen::~CodeGen()
{
}

void CodeGen::visit(meta::Function *node)
{
    buildContext->varMap.clear();
    buildContext->currFunc = nullptr;

    ChildrenGatherer<meta::Arg> argGatherer;
    node->walk(&argGatherer);

    llvm::Type *intType = llvm::Type::getInt32Ty(globalContext);
    std::vector<llvm::Type *> argTypes;
    for (const auto arg : argGatherer.gathered()) {
        if (arg->type() == "int")
            argTypes.push_back(intType);
        else
            throw std::runtime_error(std::string("Argument ") + arg->name() + " of the function " + node->name() + " is of unknown type " + arg->type()); /// @todo missing code position info
    }
    llvm::FunctionType *funcType = llvm::FunctionType::get(intType, argTypes, false);
    buildContext->currFunc = llvm::Function::Create(funcType, llvm::GlobalValue::ExternalLinkage, node->name(), module.get());
    llvm::Function::arg_iterator it = buildContext->currFunc->arg_begin();
    for (const auto arg : argGatherer.gathered()) {
        it->setName(arg->name());
        buildContext->varMap[arg->name()] = it; /// @todo check name collisions
        assert(it != buildContext->currFunc->arg_end());
        ++it;
    }
    assert(it == buildContext->currFunc->arg_end());

    llvm::BasicBlock *body = llvm::BasicBlock::Create(globalContext, "", buildContext->currFunc);
    buildContext->builder.SetInsertPoint(body);
}

void CodeGen::visit(meta::Call *)
{
    /// @todo add call functions support
    throw std::runtime_error("Not implemented yet");
}

void CodeGen::visit(meta::Number *node)
{
    llvm::Type *intType = llvm::Type::getInt32Ty(globalContext);
    buildContext->evaluationStack.push(llvm::ConstantInt::get(intType, node->value(), true));
}

void CodeGen::visit(meta::Var *node)
{
    auto it = buildContext->varMap.find(node->name());
    if (it == buildContext->varMap.end())
        throw std::runtime_error(std::string("Undefined variable ") + node->name());
    buildContext->evaluationStack.push(it->second);
}

void CodeGen::leave(meta::BinaryOp* node)
{
    assert(buildContext->evaluationStack.size() >=2);
    llvm::Value *right = buildContext->evaluationStack.top();
    buildContext->evaluationStack.pop();
    llvm::Value *left = buildContext->evaluationStack.top();
    switch (node->operation()) {
        case meta::BinaryOp::add: buildContext->evaluationStack.top() = buildContext->builder.CreateAdd(left, right); break;
        case meta::BinaryOp::sub: buildContext->evaluationStack.top() = buildContext->builder.CreateSub(left, right); break;
        case meta::BinaryOp::mul: buildContext->evaluationStack.top() = buildContext->builder.CreateMul(left, right); break;
        case meta::BinaryOp::div: buildContext->evaluationStack.top() = buildContext->builder.CreateSDiv(left, right); break;
        default: assert(false);
    }
}

void CodeGen::leave(meta::Return* node)
{
    assert(buildContext->evaluationStack.size() == 1);
    buildContext->builder.CreateRet(buildContext->evaluationStack.top());
    buildContext->evaluationStack.pop();
}

void CodeGen::save(const std::string& path)
{
    std::string errBuf;
    llvm::raw_fd_ostream out(path.c_str(), errBuf, llvm::sys::fs::F_Binary);
    llvm::WriteBitcodeToFile(module.get(), out);
    out.close();
    if (out.has_error())
        throw std::runtime_error(errBuf);
}
