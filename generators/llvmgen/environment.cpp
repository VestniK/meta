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

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <stdexcept>

#include "parser/metanodes.h"

#include "typesystem/type.h"

#include "generators/llvmgen/mangling.h"
#include "generators/llvmgen/environment.h"

namespace meta {
namespace generators {
namespace llvmgen {

Environment::Environment(const std::string& moduleName):
    context(llvm::getGlobalContext()),
    module(new llvm::Module(moduleName, context)),
    string(llvm::StructType::get(
        llvm::Type::getInt32PtrTy(context), // refcount ptr
        llvm::Type::getInt8PtrTy(context), // content ptr
        llvm::Type::getInt32Ty(context), // size
    nullptr))
{
}

Environment::~Environment()
{
}

llvm::Function *Environment::addFunction(Function *func)
{
    const auto args = func->args();
    std::vector<llvm::Type *> argTypes;
    for (const auto arg : args) {
        assert(arg->type() != nullptr); // must be set during type integryty checks
        auto type = getType(arg->type());
        assert(type != nullptr); // all known types should be mapped to llvm types
        argTypes.push_back(type);
    }
    auto rettype = getType(func->type());
    assert(rettype != nullptr);
    llvm::FunctionType *funcType = llvm::FunctionType::get(rettype, argTypes, false);
    const llvm::GlobalValue::LinkageTypes linkType = func->visibility() == Visibility::Export || func->visibility() == Visibility::Extern ?
        llvm::GlobalValue::ExternalLinkage :
        llvm::GlobalValue::PrivateLinkage
    ;
    llvm::Function *prototype = llvm::Function::Create(funcType, linkType, mangledName(func), module.get());
    llvm::Function::arg_iterator it = prototype->arg_begin();
    for (const auto arg : args) {
        it->setName(llvm::StringRef(arg->name().data(), arg->name().size()));
        assert(it != prototype->arg_end());
        ++it;
    }
    assert(it == prototype->arg_end());
    return prototype;
}

llvm::Type *Environment::getType(const typesystem::Type *type)
{
    // built in types:
    switch (type->typeId()) {
        case typesystem::Type::Int: return llvm::Type::getInt32Ty(context);
        case typesystem::Type::Bool: return llvm::Type::getInt1Ty(context);
        case typesystem::Type::String: return string;

        case typesystem::Type::Auto: assert(false);
        case typesystem::Type::Void: return llvm::Type::getVoidTy(context);
    }
    return nullptr;
}

} // namespace llvmgen
} // namespace generators
} // namespace meta
