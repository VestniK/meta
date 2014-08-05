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

struct ModuleBuilderPrivate
{
    ModuleBuilderPrivate(Environment &env): env(env), builder(env.context) {}

    void function(meta::Function *node);
    void varDecl(meta::VarDecl *node, llvm::Value *initialVal);
    void returnVal(meta::Return *node, llvm::Value *val);
    llvm::Value *call(meta::Call *node, const std::vector<llvm::Value*> &args);
    llvm::Value *number(meta::Number *node);
    llvm::Value *var(meta::Var *node);
    llvm::Value *assign(meta::Assigment *node, llvm::Value *val);
    llvm::Value *binaryOp(meta::BinaryOp *node, llvm::Value *left, llvm::Value *right);

    Environment &env;
    std::map<std::string, llvm::Value*> regVarMap; // IR registry allocated
    std::map<std::string, llvm::AllocaInst*> stackVarMap; // Stack allocated
    llvm::IRBuilder<> builder;
};

void ModuleBuilderPrivate::function(meta::Function *node)
{
    regVarMap.clear();
    stackVarMap.clear();
    llvm::Function *func = env.module->getFunction(node->name());
    assert(func); // All functions should be registered in the environment before generation

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

void ModuleBuilderPrivate::varDecl(meta::VarDecl *node, llvm::Value *initialVal)
{
    llvm::Function *currFunc = builder.GetInsertBlock()->getParent();
    llvm::IRBuilder<> stackVarDeclBuilder(&(currFunc->getEntryBlock()), currFunc->getEntryBlock().begin());
    llvm::Type *type = llvm::Type::getInt32Ty(env.context); // TODO use node->type() to calculate properly
    llvm::AllocaInst *stackVar = stackVarDeclBuilder.CreateAlloca(type, 0, node->name().c_str());

    stackVarMap[node->name()] = stackVar;

    if (node->inited())
        builder.CreateStore(initialVal, stackVar);
}

void ModuleBuilderPrivate::returnVal(meta::Return *node, llvm::Value *val)
{
    builder.CreateRet(val);
}

llvm::Value *ModuleBuilderPrivate::call(meta::Call *node, const std::vector<llvm::Value*> &args)
{
    llvm::Function *func = env.module->getFunction(node->functionName());
    /// @todo this kind of checks should be done before generation
    if (func == nullptr)
        throw std::runtime_error(std::string("Call of unknown function " + node->functionName()));
    if (func->arg_size() != args.size())
        throw std::runtime_error(std::string("Call of the function ") + node->functionName() + " with incorrect number of arguments");
    return builder.CreateCall(func, args);
}

llvm::Value *ModuleBuilderPrivate::number(meta::Number *node)
{
    llvm::Type *intType = llvm::Type::getInt32Ty(env.context);
    return llvm::ConstantInt::get(intType, node->value(), true);
}

llvm::Value *ModuleBuilderPrivate::var(meta::Var *node)
{
    auto itReg = regVarMap.find(node->name());
    if (itReg != regVarMap.end())
        return itReg->second;

    auto itStack = stackVarMap.find(node->name());
    if (itStack == stackVarMap.end())
        throw std::runtime_error(std::string("Undefined variable ") + node->name());
    return builder.CreateLoad(itStack->second);
}

llvm::Value *ModuleBuilderPrivate::assign(meta::Assigment *node, llvm::Value *val)
{
    auto it = stackVarMap.find(node->varName());
    if (it == stackVarMap.end())
        throw std::runtime_error(std::string("Variable ") + node->varName() + " doesn't exists or not mutable");
    builder.CreateStore(val, it->second);
    return val;
}

llvm::Value *ModuleBuilderPrivate::binaryOp(meta::BinaryOp *node, llvm::Value *left, llvm::Value *right)
{
    switch (node->operation()) {
        case meta::BinaryOp::add: return builder.CreateAdd(left, right);
        case meta::BinaryOp::sub: return builder.CreateSub(left, right);
        case meta::BinaryOp::mul: return builder.CreateMul(left, right);
        case meta::BinaryOp::div: return builder.CreateSDiv(left, right);
        default: assert(false);
    }
    return nullptr;
}

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
    d(new ModuleBuilderPrivate(env))
{
}

ModuleBuilder::~ModuleBuilder()
{
}

void ModuleBuilder::visit(meta::Function *node)
{
    d->function(node);
}

void ModuleBuilder::visit(meta::Call *node)
{
    push(evaluationStack, nullptr, node->functionName() + " call result placeholder");
}

void ModuleBuilder::leave(meta::Call *node)
{
    std::vector<llvm::Value*> args;
    for (; evaluationStack.top() != nullptr; pop(evaluationStack, "call argument"))
        args.push_back(evaluationStack.top());
    set(evaluationStack, d->call(node, args), "call result placeholder", node->functionName() + " result");
}

void ModuleBuilder::visit(meta::Number *node)
{
    push(evaluationStack, d->number(node), "number constant");
}

void ModuleBuilder::visit(meta::Var *node)
{
    push(evaluationStack, d->var(node), node->name() + " variable value");
}

void ModuleBuilder::leave(meta::VarDecl *node)
{
    assert(evaluationStack.size() == node->inited() ? 1 : 0);
    d->varDecl(node, node->inited() ? evaluationStack.top() : nullptr);
    if (node->inited())
        pop(evaluationStack, node->name() + " var initial value");
}

void ModuleBuilder::leave(meta::Assigment *node)
{
    assert(evaluationStack.size() == 1);
    set(evaluationStack, d->assign(node, evaluationStack.top()), "assigment rval", "assigment result");
}

void ModuleBuilder::leave(meta::BinaryOp *node)
{
    assert(evaluationStack.size() >=2);
    llvm::Value *right = evaluationStack.top();
    pop(evaluationStack, "bin op right operand");
    llvm::Value *left = evaluationStack.top();
    set(evaluationStack, d->binaryOp(node, left, right), "left operand", "binary op result");
}

void ModuleBuilder::leave(meta::Return *node)
{
    assert(evaluationStack.size() == 1);
    d->returnVal(node, evaluationStack.top());
    pop(evaluationStack, "return value");
}

void ModuleBuilder::leave(meta::ExprStatement *)
{
    assert(evaluationStack.size() == 1);
    pop(evaluationStack, "unused expr result");
}

void ModuleBuilder::save(const std::string& path)
{
    std::string errBuf;
    llvm::raw_fd_ostream out(path.c_str(), errBuf, llvm::sys::fs::F_Binary);
    llvm::WriteBitcodeToFile(d->env.module.get(), out);
    out.close();
    if (out.has_error())
        throw std::runtime_error(errBuf);
}

} // namespace llvmgen
} // namespace generators
