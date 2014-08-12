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

void ModuleBuilder::startFunction(meta::Function *node)
{
    mVarMap.clear();
    llvm::Function *func = env.module->getFunction(node->name());
    if (!func)
        func = env.addFunction(node);

    llvm::Function::arg_iterator it = func->arg_begin();
    for (const auto arg : node->args()) {
        mVarMap[arg] = it;
        assert(it != func->arg_end());
        ++it;
    }
    assert(it == func->arg_end());

    llvm::BasicBlock *body = llvm::BasicBlock::Create(env.context, "", func);
    builder.SetInsertPoint(body);
}

void ModuleBuilder::returnValue(meta::Return *node, llvm::Value *val)
{
    builder.CreateRet(val);
}

llvm::Value *ModuleBuilder::call(meta::Call *node, const std::vector<llvm::Value*> &args)
{
    llvm::Function *func = env.module->getFunction(node->functionName());
    if (!func) {
        assert(node->function() != nullptr);
        func = env.addFunction(node->function());
    }
    assert(func->arg_size() == args.size());
    return builder.CreateCall(func, args);
}

llvm::Value *ModuleBuilder::number(meta::Number *node)
{
    llvm::Type *intType = llvm::Type::getInt32Ty(env.context);
    return llvm::ConstantInt::get(intType, node->value(), true);
}

llvm::Value *ModuleBuilder::var(meta::Var *node)
{
    assert(node->declaration());
    auto it = mVarMap.find(node->declaration());
    assert(it != mVarMap.end()); // Use befor initialization should be checked by analizers

    return node->declaration()->is(meta::VarDecl::argument) ? it->second : builder.CreateLoad(it->second);
}

llvm::Value *ModuleBuilder::assign(meta::VarDecl *node, llvm::Value *val)
{
    assert(!node->is(meta::VarDecl::argument));
    auto it = mVarMap.find(node);
    if (it == mVarMap.end()) {
        llvm::Function *currFunc = builder.GetInsertBlock()->getParent();
        llvm::IRBuilder<> stackVarDeclBuilder(&(currFunc->getEntryBlock()), currFunc->getEntryBlock().begin());
        llvm::Type *type = llvm::Type::getInt32Ty(env.context); // TODO use node->type() to calculate properly
        mVarMap[node] = stackVarDeclBuilder.CreateAlloca(type, 0, node->name().c_str());
        it = mVarMap.find(node);
    }
    builder.CreateStore(val, it->second);
    return val;
}

llvm::Value *ModuleBuilder::binaryOp(meta::BinaryOp *node, llvm::Value *left, llvm::Value *right)
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

llvm::Value *ModuleBuilder::prefixOp(meta::PrefixOp *node, llvm::Value *val)
{
    switch (node->operation()) {
        case meta::PrefixOp::negative: return builder.CreateNeg(val);
        case meta::PrefixOp::positive: return val;
        default: assert(false);
    }
    return nullptr;
}

void ModuleBuilder::save(const std::string &path)
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
