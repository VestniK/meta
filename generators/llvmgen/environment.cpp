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

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <stdexcept>

#include "parser/metanodes.h"

#include "generators/llvmgen/environment.h"

namespace generators {
namespace llvmgen {

Environment::Environment(const std::string& moduleName):
    context(llvm::getGlobalContext()),
    module(new llvm::Module(moduleName, context))
{
}

Environment::~Environment()
{
}

llvm::Function *Environment::addFunction(meta::Function *func)
{
    const auto args = func->args();
    llvm::Type *intType = llvm::Type::getInt32Ty(context);
    std::vector<llvm::Type *> argTypes;
    for (const auto arg : args) {
        if (arg->type() == "int")
            argTypes.push_back(intType);
        else
            throw std::runtime_error(std::string("Argument ") + arg->name() + " of the function " + func->name() + " is of unknown type " + arg->type()); /// @todo missing code position info
    }
    llvm::FunctionType *funcType = llvm::FunctionType::get(intType, argTypes, false);
    llvm::Function *prototype = llvm::Function::Create(funcType, llvm::GlobalValue::ExternalLinkage, func->name(), module.get());
    llvm::Function::arg_iterator it = prototype->arg_begin();
    for (const auto arg : args) {
        it->setName(arg->name());
        assert(it != prototype->arg_end());
        ++it;
    }
    assert(it == prototype->arg_end());
    return prototype;
}

} // namespace llvmgen
} // namespace generators

