/*
 * Meta language compiler
 * Copyright (C) 2014  Sergey Vidyuk <sir.vestnik@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
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

#include "generators/llvmgen/expressionbuilder.h"
#include "generators/llvmgen/modulebuilder.h"
#include "generators/abi/mangling.h"

namespace meta {
namespace generators {
namespace llvmgen {

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
