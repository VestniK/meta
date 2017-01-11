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

Environment::Environment(utils::string_view moduleName):
    context(),
    module(new llvm::Module({moduleName.data(), moduleName.size()}, context)),
    string(llvm::StructType::get(
        llvm::Type::getInt32PtrTy(context), // control block pointer used by meta-rt only
        llvm::Type::getInt8PtrTy(context), // content ptr
        llvm::Type::getInt32Ty(context), // size
    nullptr))
{
}

llvm::Function *Environment::addFunction(Function *func) {
    const auto args = func->args();
    std::vector<llvm::Type *> argTypes;
    auto rettype = getType(func->type());
    assert(rettype != nullptr);
    if (func->type() & typesystem::TypeProp::sret)
        argTypes.push_back(rettype->getPointerTo());
    for (const auto arg : args) {
        assert(arg->type() != nullptr); // must be set during type integryty checks
        auto type = getType(arg->type());
        assert(type != nullptr); // all known types should be mapped to llvm types
        argTypes.push_back(type);
    }
    llvm::FunctionType *funcType = llvm::FunctionType::get(
        (func->type() & typesystem::TypeProp::sret) ? llvm::Type::getVoidTy(context) : rettype,
        argTypes, false
    );
    const llvm::GlobalValue::LinkageTypes linkType = func->visibility() == Visibility::Export || func->visibility() == Visibility::Extern ?
        llvm::GlobalValue::ExternalLinkage :
        llvm::GlobalValue::PrivateLinkage
    ;
    llvm::Function *prototype = llvm::Function::Create(funcType, linkType, mangledName(func), module.get());
    llvm::Function::arg_iterator it = prototype->arg_begin();
    if (func->type() & typesystem::TypeProp::sret) {
        llvm::AttrBuilder attrBuilder;
        attrBuilder.addAttribute(llvm::Attribute::StructRet);
        it->addAttr(llvm::AttributeSet::get(context, 0, attrBuilder));
        ++it;
    }
    for (const auto arg : args) {
        it->setName(llvm::StringRef(arg->name().data(), arg->name().size()));
        assert(it != prototype->arg_end());
        ++it;
    }
    assert(it == prototype->arg_end());
    return prototype;
}

llvm::Type* Environment::getType(const typesystem::Type* type) {
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

llvm::AllocaInst* addLocalVar(llvm::Function* func, llvm::Type* type, utils::string_view name) {
    llvm::IRBuilder<> builder(&(func->getEntryBlock()), func->getEntryBlock().begin());
    return builder.CreateAlloca(type, 0, llvm::StringRef(name.data(), name.size()));
}

} // namespace llvmgen
} // namespace generators
} // namespace meta
