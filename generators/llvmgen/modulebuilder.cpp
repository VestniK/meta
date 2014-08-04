#include <cassert>
#include <cstdio>
#include <stdexcept>
#include <vector>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/raw_ostream.h>

#include "generators/llvmgen/modulebuilder.h"

namespace generators {
namespace llvmgen {

static bool stackLogEnabled = false;

static inline void push(std::stack<llvm::Value*> &stack, llvm::Value *val, const std::string &what)
{
    if (stackLogEnabled) fprintf(stderr, "PUSH: %s\n", what.c_str());
    stack.push(val);
}

static inline void pop(std::stack<llvm::Value*> &stack, const std::string &what)
{
    if (stackLogEnabled) fprintf(stderr, "POP: %s\n", what.c_str());
    stack.pop();
}

static inline void set(std::stack<llvm::Value*> &stack, llvm::Value *val, const std::string &from, const std::string &to)
{
    if (stackLogEnabled) fprintf(stderr, "CHANGE: %s with %s\n", from.c_str(), to.c_str());
    stack.top() = val;
}

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

void ModuleBuilder::visit(meta::Call *node)
{
    push(evaluationStack, nullptr, node->functionName() + " call result placeholder");
}

void ModuleBuilder::leave(meta::Call *node)
{
    llvm::Function *func = env.module->getFunction(node->functionName());
    /// @todo this kind of checks should be done before compilation
    if (func == nullptr)
        throw std::runtime_error(std::string("Call of unknown function " + node->functionName()));
    std::vector<llvm::Value*> args;
    for (; evaluationStack.top() != nullptr; pop(evaluationStack, "call argument"))
        args.push_back(evaluationStack.top());
    if (func->arg_size() != args.size())
        throw std::runtime_error(std::string("Call of the function ") + node->functionName() + " with incorrect number of arguments");
    set(evaluationStack, builder.CreateCall(func, args), "call result placeholder", node->functionName() + " result");
}

void ModuleBuilder::visit(meta::Number *node)
{
    llvm::Type *intType = llvm::Type::getInt32Ty(env.context);
    push(evaluationStack, llvm::ConstantInt::get(intType, node->value(), true), "number constant");
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
    push(evaluationStack, val, node->name() + " variable value");
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
    pop(evaluationStack, node->name() + " var initial value");
}

void ModuleBuilder::leave(meta::Assigment *node)
{
    auto it = stackVarMap.find(node->varName());
    if (it == stackVarMap.end())
        throw std::runtime_error(std::string("Variable ") + node->varName() + " doesn't exists or not mutable");
    assert(evaluationStack.size() >= 1);
    builder.CreateStore(evaluationStack.top(), it->second);
    pop(evaluationStack, node->varName() + " var new value");
}

void ModuleBuilder::leave(meta::BinaryOp *node)
{
    assert(evaluationStack.size() >=2);
    llvm::Value *right = evaluationStack.top();
    pop(evaluationStack, "bin op right operand");
    llvm::Value *left = evaluationStack.top();
    switch (node->operation()) {
        case meta::BinaryOp::add: set(evaluationStack, builder.CreateAdd(left, right), "left operand", "operator+ value"); break;
        case meta::BinaryOp::sub: set(evaluationStack, builder.CreateSub(left, right), "left operand", "operator- value"); break;
        case meta::BinaryOp::mul: set(evaluationStack, builder.CreateMul(left, right), "left operand", "operator* value"); break;
        case meta::BinaryOp::div: set(evaluationStack, builder.CreateSDiv(left, right), "left operand", "operator/ value"); break;
        default: assert(false);
    }
}

void ModuleBuilder::leave(meta::Return *node)
{
    assert(evaluationStack.size() == 1);
    builder.CreateRet(evaluationStack.top());
    pop(evaluationStack, "return value");
}

void ModuleBuilder::leave(meta::ExprStatement *)
{
    pop(evaluationStack, "unused expr result");
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
