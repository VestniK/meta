/*
 * Meta language compiler
 * Copyright (C) 2015  Sergey Vidyuk <sir.vestnik@gmail.com>
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
 */
#include <cassert>

#include <llvm/IR/Module.h>

#include "utils/contract.h"

#include "generators/abi/mangling.h"
#include "generators/llvmgen/environment.h"
#include "generators/llvmgen/expressionbuilder.h"

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

} // namespace llvmgen
} // namespace generators
} // namespace meta
