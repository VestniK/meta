#include <cassert>
#include <stdexcept>
#include <vector>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/raw_ostream.h>

#include "generators/llvmgen/modulebuilder.h"

namespace generators {
namespace llvmgen {

ModuleBuilder::ModuleBuilder(Environment &env):
    env(env),
    builder(env.context)
{
}

ModuleBuilder::~ModuleBuilder()
{
}

void ModuleBuilder::visit(meta::Function *node)
{
    regVarMap.clear();
    stackVarMap.clear();
    llvm::Function *func = env.module->getFunction(node->name());
    assert(func); // All functions should be registered in the environment before compilation

    llvm::Function::arg_iterator it = func->arg_begin();
    for (const auto arg : node->args()) {
        regVarMap[arg->name()] = it; /// @todo check name collisions
        assert(it != func->arg_end());
        ++it;
    }
    assert(it == func->arg_end());

    llvm::BasicBlock *body = llvm::BasicBlock::Create(env.context, "", func);
    builder.SetInsertPoint(body);
}

void ModuleBuilder::visit(meta::Call *)
{
    evaluationStack.push(nullptr); // Marker for the result operation value
}

void ModuleBuilder::leave(meta::Call *node)
{
    llvm::Function *func = env.module->getFunction(node->functionName());
    /// @todo this kind of checks should be done before compilation
    if (func == nullptr)
        throw std::runtime_error(std::string("Call of unknown function " + node->functionName()));
    std::vector<llvm::Value*> args;
    for (; evaluationStack.top() != nullptr; evaluationStack.pop())
        args.push_back(evaluationStack.top());
    if (func->arg_size() != args.size())
        throw std::runtime_error(std::string("Call of the function ") + node->functionName() + " with incorrect number of arguments");
    evaluationStack.top() = builder.CreateCall(func, args);
}

void ModuleBuilder::visit(meta::Number *node)
{
    llvm::Type *intType = llvm::Type::getInt32Ty(env.context);
    evaluationStack.push(llvm::ConstantInt::get(intType, node->value(), true));
}

void ModuleBuilder::visit(meta::Var *node)
{
    auto it = regVarMap.find(node->name());
    llvm::Value *val = nullptr;
    if (it != regVarMap.end())
        val = it->second;
    else {
        auto it = stackVarMap.find(node->name());
        if (it == stackVarMap.end())
            throw std::runtime_error(std::string("Undefined variable ") + node->name());
        val = builder.CreateLoad(it->second);
    }
    evaluationStack.push(val);
}

void ModuleBuilder::leave(meta::VarDecl *node)
{
    llvm::Function *currFunc = builder.GetInsertBlock()->getParent();
    llvm::IRBuilder<> stackVarDeclBuilder(&(currFunc->getEntryBlock()), currFunc->getEntryBlock().begin());
    llvm::Type *type = llvm::Type::getInt32Ty(env.context); // TODO use note->type() to calculate properly
    llvm::AllocaInst *stackVar = stackVarDeclBuilder.CreateAlloca(type, 0, node->name().c_str());

    stackVarMap[node->name()] = stackVar;

    if (!node->inited())
        return;
    assert(evaluationStack.size() >= 1);
    builder.CreateStore(evaluationStack.top(), stackVar);
    evaluationStack.pop();
}

void ModuleBuilder::leave(meta::Assigment *node)
{
    auto it = stackVarMap.find(node->varName());
    if (it == stackVarMap.end())
        throw std::runtime_error(std::string("Variable ") + node->varName() + " doesn't exists or not mutable");
    assert(evaluationStack.size() >= 1);
    builder.CreateStore(evaluationStack.top(), it->second);
    evaluationStack.pop();
}

void ModuleBuilder::leave(meta::BinaryOp *node)
{
    assert(evaluationStack.size() >=2);
    llvm::Value *right = evaluationStack.top();
    evaluationStack.pop();
    llvm::Value *left = evaluationStack.top();
    switch (node->operation()) {
        case meta::BinaryOp::add: evaluationStack.top() = builder.CreateAdd(left, right); break;
        case meta::BinaryOp::sub: evaluationStack.top() = builder.CreateSub(left, right); break;
        case meta::BinaryOp::mul: evaluationStack.top() = builder.CreateMul(left, right); break;
        case meta::BinaryOp::div: evaluationStack.top() = builder.CreateSDiv(left, right); break;
        default: assert(false);
    }
}

void ModuleBuilder::leave(meta::Return *node)
{
    assert(evaluationStack.size() == 1);
    builder.CreateRet(evaluationStack.top());
    evaluationStack.pop();
}

void ModuleBuilder::save(const std::string& path)
{
    std::string errBuf;
    llvm::raw_fd_ostream out(path.c_str(), errBuf, llvm::sys::fs::F_Binary);
    llvm::WriteBitcodeToFile(env.module.get(), out);
    out.close();
    if (out.has_error())
        throw std::runtime_error(errBuf);
}

} // namespace llvmgen
} // namespace generators
