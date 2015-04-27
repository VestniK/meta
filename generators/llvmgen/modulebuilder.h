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

#include "generators/llvmgen/environment.h"
#include "generators/llvmgen/privateheadercheck.h"

namespace meta {
namespace generators {
namespace llvmgen {

class ModuleBuilder: public Visitor
{
public:
    ModuleBuilder(Environment &env): env(env), builder(env.context) {}

    bool visit(Function *node) override;
    bool visit(VarDecl *node) override;
    bool visit(Return *node) override;
    bool visit(If *node) override;
    bool visit(ExprStatement *node) override;
    bool visit(CodeBlock *node) override;

    void save(const std::string &path);

private:
    Environment &env;
    std::map<VarDecl *, llvm::Value *> mVarMap;
    llvm::IRBuilder<> builder;
    bool mCurrBlockTerminated = false;
};

} // namespace llvmgen
} // namespace generators
} // namespace meta
