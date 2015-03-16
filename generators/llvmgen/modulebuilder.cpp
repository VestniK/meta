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
    mCurrBlockTerminated = false;
    mVarMap.clear();
    llvm::Function *func = env.module->getFunction(generators::abi::mangledName(node));
    if (!func)
        func = env.addFunction(node);
    if (node->visibility() == meta::Visibility::Extern)
        return false;

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
    mCurrBlockTerminated = true;
}

void ModuleBuilder::returnVoid(meta::Return *node)
{
    builder.CreateRetVoid();
    mCurrBlockTerminated = true;
}

void ModuleBuilder::ifCond(meta::If *node, llvm::Value *val)
{
    if (!node->thenBlock() && !node->elseBlock()) // "if (cond) ;" || "if (cond) ; else ;" no additional generation needed
        return;
    llvm::Function *func = builder.GetInsertBlock()->getParent();
    auto mergeBB = llvm::BasicBlock::Create(env.context, "merge");
    auto thenBB = node->thenBlock() ? llvm::BasicBlock::Create(env.context, "then") : mergeBB;
    auto elseBB = node->elseBlock() ? llvm::BasicBlock::Create(env.context, "else") : mergeBB;

    builder.CreateCondBr(val, thenBB, elseBB);

    if (node->thenBlock()) {
        func->getBasicBlockList().push_back(thenBB);
        builder.SetInsertPoint(thenBB);
        node->thenBlock()->walk(this);
        if (!mCurrBlockTerminated)
            builder.CreateBr(mergeBB);
        mCurrBlockTerminated = false;
    }
    if (node->elseBlock()) {
        func->getBasicBlockList().push_back(elseBB);
        builder.SetInsertPoint(elseBB);
        node->elseBlock()->walk(this);
        if (!mCurrBlockTerminated)
            builder.CreateBr(mergeBB);
        mCurrBlockTerminated = false;
    }
    func->getBasicBlockList().push_back(mergeBB);
    builder.SetInsertPoint(mergeBB);
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
    llvm::Type *type = env.getType(node->type());
    return llvm::ConstantInt::get(type, node->value(), true);
}

llvm::Value *ModuleBuilder::literal(meta::Literal *node)
{
    llvm::Type *type = env.getType(node->type());
    switch (node->value()) {
        case meta::Literal::trueVal: return llvm::ConstantInt::getTrue(type);
        case meta::Literal::falseVal:return llvm::ConstantInt::getFalse(type);
    }
    assert(false);
    return nullptr;
}

llvm::Value *ModuleBuilder::strLiteral(meta::StrLiteral *node)
{
    // TODO: better string needed
    return llvm::ConstantDataArray::getString(env.context, llvm::StringRef(node->value().data(), node->value().size()));
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
    auto type = env.getType(node->type());
    assert(type != nullptr); // types integrity should be checked by analyzers
    auto allocaVal = mVarMap[node] = addLocalVar(builder.GetInsertBlock()->getParent(), type, node->name());
    builder.CreateStore(val, allocaVal);
}

llvm::Value *ModuleBuilder::assign(meta::Assigment *node, llvm::Value *val)
{
    assert(!node->declaration()->is(meta::VarDecl::argument));
    auto it = mVarMap.find(node->declaration());
    if (it == mVarMap.end()) {
        auto type = env.getType(node->declaration()->type());
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

        case meta::BinaryOp::equal: return builder.CreateICmpEQ(left, right);
        case meta::BinaryOp::noteq: return builder.CreateICmpNE(left, right);

        case meta::BinaryOp::less: return builder.CreateICmpSLT(left, right);
        case meta::BinaryOp::lesseq: return builder.CreateICmpSLE(left, right);
        case meta::BinaryOp::greater: return builder.CreateICmpSGT(left, right);
        case meta::BinaryOp::greatereq: return builder.CreateICmpSGE(left, right);

        case meta::BinaryOp::boolAnd: return builder.CreateAnd(left, right);
        case meta::BinaryOp::boolOr: return builder.CreateOr(left,right);
    }
    assert(false);
    return nullptr;
}

llvm::Value *ModuleBuilder::prefixOp(meta::PrefixOp *node, llvm::Value *val)
{
    switch (node->operation()) {
        case meta::PrefixOp::negative: return builder.CreateNeg(val);
        case meta::PrefixOp::positive: return val;
        case meta::PrefixOp::boolnot: return builder.CreateNot(val);
    }
    assert(false);
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
