#include <cassert>
#include <cstdio>
#include <stdexcept>
#include <vector>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

#include "generators/llvmgen/modulebuilder.h"
#include "generators/abi/mangling.h"

namespace generators {
namespace llvmgen {

bool ModuleBuilder::visit(meta::Function *node)
{
    mVarMap.clear();
    llvm::Function *func = env.module->getFunction(generators::abi::mangledName(node));
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
    return true;
}

void ModuleBuilder::returnValue(meta::Return *node, llvm::Value *val)
{
    builder.CreateRet(val);
}

llvm::Value *ModuleBuilder::call(meta::Call *node, const std::vector<llvm::Value*> &args)
{
    llvm::Function *func = env.module->getFunction(generators::abi::mangledName(node->function()));
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

namespace {
inline llvm::AllocaInst *addLocalVar(llvm::Function *func, llvm::Type *type, const std::string &name)
{
    llvm::IRBuilder<> builder(&(func->getEntryBlock()), func->getEntryBlock().begin());
    return builder.CreateAlloca(type, 0, name.c_str());
}
}

void ModuleBuilder::varInit(meta::VarDecl *node, llvm::Value *val)
{
    auto type = env.getType(node->typeName());
    assert(type != nullptr); // types integrity should be checked by analyzers
    auto allocaVal = mVarMap[node] = addLocalVar(builder.GetInsertBlock()->getParent(), type, node->name());
    builder.CreateStore(val, allocaVal);
}

llvm::Value *ModuleBuilder::assign(meta::Assigment *node, llvm::Value *val)
{
    assert(!node->declaration()->is(meta::VarDecl::argument));
    auto it = mVarMap.find(node->declaration());
    if (it == mVarMap.end()) {
        auto type = env.getType(node->declaration()->typeName());
        assert(type != nullptr); // types integrity should be checked by analyzers
        mVarMap[node->declaration()] = addLocalVar(builder.GetInsertBlock()->getParent(), type, node->declaration()->name());
        it = mVarMap.find(node->declaration());
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
    llvm::raw_fd_ostream out(path.c_str(), errBuf, llvm::sys::fs::F_None);
    llvm::WriteBitcodeToFile(env.module.get(), out);
    out.close();
    if (out.has_error())
        throw std::runtime_error(errBuf);
}

} // namespace llvmgen
} // namespace generators
