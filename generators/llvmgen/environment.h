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
#include <memory>

#include <llvm/IR/IRBuilder.h>

#include "utils/types.h"

#include "generators/llvmgen/privateheadercheck.h"

namespace llvm {

class LLVMContext;
class Module;
class Type;
class StructType;
class Value;

}

namespace meta {

class Function;
class VarDecl;

namespace typesystem {

class Type;

} // namespace typesystem

namespace generators {
namespace llvmgen {

struct Environment {
    Environment(utils::string_view moduleName);

    llvm::Function* addFunction(Function* func);
    llvm::Type* getType(const typesystem::Type* type);

    llvm::LLVMContext context;
    std::unique_ptr<llvm::Module> module;
    llvm::StructType* string;
};

struct Context {
    Environment &env;
    std::map<VarDecl*, llvm::Value*> varMap;
    llvm::IRBuilder<> builder;
};

llvm::AllocaInst* addLocalVar(llvm::Function* func, llvm::Type* type, utils::string_view name);

} // namespace llvmgen
} // namespace generators
} // namespace meta
