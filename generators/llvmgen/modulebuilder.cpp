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
#include <sstream>
#include <stdexcept>
#include <system_error>
#include <vector>

#include <llvm/Analysis/CGSCCPassManager.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/raw_os_ostream.h>

#include "utils/contract.h"

#include "typesystem/type.h"

#include "analysers/semanticerror.h"

#include "generators/llvmgen/expressionbuilder.h"
#include "generators/llvmgen/modulebuilder.h"
#include "generators/llvmgen/fixstructretpass.h"
#include "generators/llvmgen/mangling.h"

namespace meta::generators::llvmgen {

bool ModuleBuilder::visit(Function *node)
{
    mCtx.varMap.clear();
    llvm::Function *func = mCtx.env.module->getFunction(mangledName(node));
    if (!func)
        func = mCtx.env.addFunction(node);
    if (node->visibility() == Visibility::Extern)
        return false;

    llvm::Function::arg_iterator it = func->arg_begin();
    if (it->hasStructRetAttr())
        ++it;
    for (const auto arg : node->args()) {
        mCtx.varMap[arg] = &(*it);
        assert(it != func->arg_end());
        ++it;
    }
    assert(it == func->arg_end());

    llvm::BasicBlock *body = llvm::BasicBlock::Create(mCtx.env.context, "", func);
    mCtx.builder.SetInsertPoint(body);
    StatementBuilder statementBuilder;
    const ExecStatus status = statementBuilder(node->body(), mCtx);
    if (status == ExecStatus::stop) // Function body ends with terminating instruction
        return false;
    if (!(node->type() & typesystem::TypeProp::voidtype))
        throw analysers::SemanticError(node, "Non-void function ends without return");
    mCtx.builder.CreateRetVoid(); // Void function with implicit return.
    return false;
}

ExecStatus StatementBuilder::operator() (VarDecl *node, Context &ctx)
{
    PRECONDITION(!(node->flags() & VarFlags::argument));
    // types integrity should be checked by analyzers
    PRECONDITION(ctx.env.getType(node->type()) != nullptr);
    auto type = ctx.env.getType(node->type());
    // TODO: good point to check for multiple definitions
    auto allocaVal = ctx.varMap[node] = addLocalVar(ctx.builder.GetInsertBlock()->getParent(), type, node->name());
    if (!node->initExpr())
        return ExecStatus::cont;
    ExpressionBuilder evaluator;
    llvm::Value *val = dispatch(evaluator, node->initExpr(), ctx);
    ctx.builder.CreateStore(val, allocaVal);
    return ExecStatus::cont;
}

ExecStatus StatementBuilder::operator() (Return *node, Context &ctx)
{
    auto value = node->value();
    if (!value) {
        ctx.builder.CreateRetVoid();
        return ExecStatus::stop;
    }
    auto retval = dispatch(ExpressionBuilder{}, value, ctx);
    auto* typedNode = dynamic_cast<Typed*>(value);
    if (typedNode->type() & typesystem::TypeProp::sret) {
        llvm::Argument& sretArg = *ctx.builder.GetInsertBlock()->getParent()->arg_begin();
        assert(sretArg.hasStructRetAttr());
        ctx.builder.CreateStore(retval, &sretArg);
        ctx.builder.CreateRetVoid();
    } else
        ctx.builder.CreateRet(retval);
    return ExecStatus::stop;
}

ExecStatus StatementBuilder::operator() (If *node, Context &ctx)
{
    ExpressionBuilder evaluator;
    llvm::Value *val = dispatch(evaluator, node->condition(), ctx);
    if (!node->thenBlock() && !node->elseBlock()) // "if (cond) ;" || "if (cond) ; else ;" no additional generation needed
        return ExecStatus::cont;
    llvm::Function *func = ctx.builder.GetInsertBlock()->getParent();
    auto mergeBB = llvm::BasicBlock::Create(ctx.env.context, "merge");
    auto thenBB = node->thenBlock() ? llvm::BasicBlock::Create(ctx.env.context, "then") : mergeBB;
    auto elseBB = node->elseBlock() ? llvm::BasicBlock::Create(ctx.env.context, "else") : mergeBB;

    ctx.builder.CreateCondBr(val, thenBB, elseBB);

    if (node->thenBlock()) {
        func->getBasicBlockList().push_back(thenBB);
        ctx.builder.SetInsertPoint(thenBB);
        if (dispatch(*this, node->thenBlock(), ctx) != ExecStatus::stop)
            ctx.builder.CreateBr(mergeBB);
    }
    if (node->elseBlock()) {
        func->getBasicBlockList().push_back(elseBB);
        ctx.builder.SetInsertPoint(elseBB);
        if (dispatch(*this, node->elseBlock(), ctx) != ExecStatus::stop)
            ctx.builder.CreateBr(mergeBB);
    }
    func->getBasicBlockList().push_back(mergeBB);
    ctx.builder.SetInsertPoint(mergeBB);
    return ExecStatus::cont;
}

ExecStatus StatementBuilder::operator() (CodeBlock *block, Context &ctx)
{
    ExecStatus lastStatus = ExecStatus::cont;
    Node *lastStatement = nullptr;
    for (Node *statement: block->statements()) {
        assert(lastStatement || lastStatus == ExecStatus::cont);
        if (lastStatus == ExecStatus::stop)
            throw analysers::SemanticError(
                statement, "Code is unreachable due to terminating statement at position %d:%d",
                lastStatement->tokens().linenum(), lastStatement->tokens().colnum()
            );

        lastStatement = statement;
        lastStatus = dispatch(*this, statement, ctx);
    }
    return lastStatus;
}

ExecStatus StatementBuilder::operator() (ExprStatement *node, Context &ctx)
{
    ExpressionBuilder evaluator;
    dispatch(evaluator, node->expression(), ctx);
    return ExecStatus::cont;
}

class IRVerificationError: public utils::Exception {
public:
    template<typename T>
    IRVerificationError(T&& t):
        utils::Exception(utils::captureBacktrace()),
        mMsg(std::forward<T>(t))
    {}
    const char* what() const noexcept override {return mMsg.c_str();}

private:
    std::string mMsg;
};

void ModuleBuilder::save(const std::string &path) {
    // verify module IR correctness
    std::ostringstream oss;
    llvm::raw_os_ostream llvmOss(oss);
    if (llvm::verifyModule(*mCtx.env.module, &llvmOss))
        throw IRVerificationError{oss.str()};
    // ABI fixup pases
    // llvm::ModulePassManager passMgr;
    // passMgr.addPass();
    // Write IR
    llvm::ModulePassManager passMgr;
    std::error_code errCode;
    llvm::raw_fd_ostream out(path.c_str(), errCode, llvm::sys::fs::F_None);
    llvm::WriteBitcodeToFile(mCtx.env.module.get(), out);
    out.close();
    if (out.has_error())
        throw std::system_error(errCode);
}

} // namespace meta::generators::llvmgen
