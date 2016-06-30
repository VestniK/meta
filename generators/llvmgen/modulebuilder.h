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
#pragma once

#include <map>
#include <string>

#include <llvm/IR/IRBuilder.h>

#include "parser/metaparser.h"
#include "parser/metanodes.h"
#include "parser/unexpectednode.h"

#include "generators/llvmgen/environment.h"
#include "generators/llvmgen/privateheadercheck.h"

namespace meta {
namespace generators {
namespace llvmgen {

enum class ExecStatus: bool
{
    stop = true,
    cont = false
};

struct StatementBuilder {

    ExecStatus operator() (Node *node, Context &) {
        throw UnexpectedNode(node, "The node is not statement don't know how to compile it");
    }

    ExecStatus operator() (CodeBlock *node, Context &ctx);
    ExecStatus operator() (Return *node, Context &ctx);
    ExecStatus operator() (If *node, Context &ctx);
    ExecStatus operator() (ExprStatement *node, Context &ctx);

    ExecStatus operator() (VarDecl *node, Context &ctx); // TODO: think about proper way to handle declarations as statements.
};

class ModuleBuilder: public Visitor
{
public:
    ModuleBuilder(Environment &env): mCtx(Context{env, std::map<VarDecl *, llvm::Value *>{}, llvm::IRBuilder<>{env.context}}) {}

    bool visit(Function *node) override;

    void save(const std::string &path);

private:
    Context mCtx;
};

} // namespace llvmgen
} // namespace generators
} // namespace meta
