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

#include <memory>

#include "generators/llvmgen/privateheadercheck.h"

namespace llvm {

class LLVMContext;
class Module;
class Type;
class StructType;

}

namespace meta {

class Function;

namespace typesystem {

class Type;

} // namespace typesystem

namespace generators {
namespace llvmgen {

struct Environment
{
    Environment(const std::string &moduleName);
    ~Environment();

    llvm::Function *addFunction(Function *func);
    llvm::Type *getType(const typesystem::Type *type);

    llvm::LLVMContext &context;
    std::unique_ptr<llvm::Module> module;
    llvm::StructType *string;
};

} // namespace llvmgen
} // namespace generators
} // namespace meta
