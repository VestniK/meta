#include <cassert>
#include <cstdio>
#include <stdexcept>
#include <system_error>
#include <vector>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

#include "generators/llvmgen/modulebuilder.h"
#include "generators/abi/mangling.h"

namespace meta {
namespace generators {
namespace llvmgen {

llvm::Value *ExpressionBuilder::operator() (Call *node)
{
    llvm::Function *func = env.module->getFunction(generators::abi::mangledName(node->function()));
    if (!func) {
        assert(node->function() != nullptr);
        func = env.addFunction(node->function());
    }
    std::vector<llvm::Value*> args;
    for (auto argNode: node->args())
        args.push_back(dispatch(*this, argNode));
    assert(func->arg_size() == args.size());
    return builder.CreateCall(func, args);
}

llvm::Value *ExpressionBuilder::operator() (Number *node)
{
    llvm::Type *type = env.getType(node->type());
    return llvm::ConstantInt::get(type, node->value(), true);
}

llvm::Value *ExpressionBuilder::operator() (Literal *node)
{
    llvm::Type *type = env.getType(node->type());
    switch (node->value()) {
        case Literal::trueVal: return llvm::ConstantInt::getTrue(type);
        case Literal::falseVal:return llvm::ConstantInt::getFalse(type);
    }
    assert(false);
    return nullptr;
}

llvm::Value *ExpressionBuilder::operator() (StrLiteral *node)
{
    return llvm::ConstantStruct::get(env.string,
        llvm::ConstantPointerNull::get(llvm::Type::getInt32PtrTy(env.context)), // no refcounter
        llvm::ConstantDataArray::getString(env.context, llvm::StringRef(node->value().data(), node->value().size()), false), // data
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(env.context), node->value().size(), false), // size
    nullptr);
}

llvm::Value *ExpressionBuilder::operator() (Var *node)
{
    assert(node->declaration());
    auto it = varMap.find(node->declaration());
    assert(it != varMap.end()); // Use befor initialization should be checked by analizers

    return node->declaration()->is(VarDecl::argument) ? it->second : builder.CreateLoad(it->second);
}

llvm::Value *ExpressionBuilder::operator() (Assigment *node)
{
    PRECONDITION(node->declaration());
    PRECONDITION(!node->declaration()->is(VarDecl::argument));
    PRECONDITION(varMap.count(node->declaration()) == 1);
    auto it = varMap.find(node->declaration());
    llvm::Value *val = dispatch(*this, node->value());
    builder.CreateStore(val, it->second);
    return val;
}

llvm::Value *ExpressionBuilder::operator() (BinaryOp *node) // TODO
{
    llvm::Value *left = dispatch(*this, node->left());
    llvm::Value *right = dispatch(*this, node->right());
    switch (node->operation()) {
        case BinaryOp::add: return builder.CreateAdd(left, right);
        case BinaryOp::sub: return builder.CreateSub(left, right);
        case BinaryOp::mul: return builder.CreateMul(left, right);
        case BinaryOp::div: return builder.CreateSDiv(left, right);

        case BinaryOp::equal: return builder.CreateICmpEQ(left, right);
        case BinaryOp::noteq: return builder.CreateICmpNE(left, right);

        case BinaryOp::less: return builder.CreateICmpSLT(left, right);
        case BinaryOp::lesseq: return builder.CreateICmpSLE(left, right);
        case BinaryOp::greater: return builder.CreateICmpSGT(left, right);
        case BinaryOp::greatereq: return builder.CreateICmpSGE(left, right);

        case BinaryOp::boolAnd: return builder.CreateAnd(left, right);
        case BinaryOp::boolOr: return builder.CreateOr(left, right);
    }
    assert(false);
    return nullptr;
}

llvm::Value *ExpressionBuilder::operator() (PrefixOp *node) // TODO
{
    llvm::Value *val = dispatch(*this, node->operand());
    switch (node->operation()) {
        case PrefixOp::negative: return builder.CreateNeg(val);
        case PrefixOp::positive: return val;
        case PrefixOp::boolnot: return builder.CreateNot(val);
    }
    assert(false);
    return nullptr;
}

bool ModuleBuilder::visit(Function *node)
{
    mCurrBlockTerminated = false;
    mVarMap.clear();
    llvm::Function *func = env.module->getFunction(generators::abi::mangledName(node));
    if (!func)
        func = env.addFunction(node);
    if (node->visibility() == Visibility::Extern)
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

bool ModuleBuilder::visit(Return *node)
{
    mCurrBlockTerminated = true;
    auto children = node->getChildren<Node>();
    if (children.empty()) {
        builder.CreateRetVoid();
        return false;
    }
    assert(children.size() == 1);
    ExpressionBuilder evaluator = {env, mVarMap, builder};
    builder.CreateRet(dispatch(evaluator, children.front()));
    return false;
}

namespace {
inline llvm::AllocaInst *addLocalVar(llvm::Function *func, llvm::Type *type, const std::string &name)
{
    llvm::IRBuilder<> builder(&(func->getEntryBlock()), func->getEntryBlock().begin());
    return builder.CreateAlloca(type, 0, name.c_str());
}
}

bool ModuleBuilder::visit(VarDecl *node)
{
    if (node->is(VarDecl::argument))
        return false;
    auto type = env.getType(node->type());
    assert(type != nullptr); // types integrity should be checked by analyzers
    auto allocaVal = mVarMap[node] = addLocalVar(builder.GetInsertBlock()->getParent(), type, node->name());
    if (!node->initExpr())
        return false;
    ExpressionBuilder evaluator = {env, mVarMap, builder};
    llvm::Value *val = dispatch(evaluator, node->initExpr());
    builder.CreateStore(val, allocaVal);
    return false;
}

bool ModuleBuilder::visit(If *node)
{
    ExpressionBuilder evaluator = {env, mVarMap, builder};
    llvm::Value *val = dispatch(evaluator, node->condition());
    if (!node->thenBlock() && !node->elseBlock()) // "if (cond) ;" || "if (cond) ; else ;" no additional generation needed
        return false;
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
    return false;
}

bool ModuleBuilder::visit(CodeBlock *)
{
    return true; // TODO: push var map in order to pop it in a leave function
}

bool ModuleBuilder::visit(ExprStatement *node)
{
    ExpressionBuilder evaluator = {env, mVarMap, builder};
    dispatch(evaluator, node->expression());
    return false;
}

void ModuleBuilder::save(const std::string &path)
{
    std::error_code errCode;
    llvm::raw_fd_ostream out(path.c_str(), errCode, llvm::sys::fs::F_None);
    llvm::WriteBitcodeToFile(env.module.get(), out);
    out.close();
    if (out.has_error())
        throw std::system_error(errCode);
}

} // namespace llvmgen
} // namespace generators
} // namespace meta
