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
 *
 */
#pragma once

#include <map>

#include "parser/metanodes.h"
#include "parser/metaparser.h"

#include "analysers/unexpectednode.h"

namespace meta {
namespace generators {
namespace llvmgen {

struct Environment;
struct Context;

struct ExpressionBuilder
{
    llvm::Value *operator() (Node *node, Context &) {throw analysers::UnexpectedNode(node, "Can't evaluate llvm::Value for non expression node");}
    llvm::Value *operator() (Expression *node, Context &) {throw analysers::UnexpectedNode(node, "Unknown expression type");}

    // Values
    llvm::Value *operator() (Number *node, Context &ctx);
    llvm::Value *operator() (Literal *node, Context &ctx);
    llvm::Value *operator() (StrLiteral *node, Context &ctx);
    llvm::Value *operator() (Var *node, Context &ctx);

    // Operations
    llvm::Value *operator() (Call *node, Context &ctx);
    llvm::Value *operator() (Assigment *node, Context &ctx);
    llvm::Value *operator() (BinaryOp *node, Context &ctx);
    llvm::Value *operator() (PrefixOp *node, Context &ctx);
};

} // namespace llvmgen
} // namespace generators
} // namespace meta
