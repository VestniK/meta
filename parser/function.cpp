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

#include "utils/contract.h"

#include "parser/metanodes.h"

namespace meta {

namespace {

struct {
    void operator() (Node *, Declaration *) {assert(false && "must be called on meta::Annotations only");}
    void operator() (Annotation *annotation, Declaration *decl) {annotation->setTarget(decl);}
} setDeclaration;

struct {
    void operator() (Node *, VarDecl::Flags, bool = true) {assert(false && "must be called on meta::VarDecl only");}
    void operator() (VarDecl *var, VarDecl::Flags flag, bool val = true) {var->set(flag, val);}
} markVarDecl;

}

Function::Function(const StackFrame *reduction, size_t size): Visitable<Declaration, Function>(reduction, size)
{
    PRECONDITION(size == 7 || size == 8); // with or without annotations

    const bool hasAnnotations = size == 8;
    if (hasAnnotations) {
        for (auto annotataion : reduction[0].nodes)
            annotataion->dispatch<void>(setDeclaration, this);
    }
    const size_t visibilityPos = (hasAnnotations ? 1 : 0);
    const size_t typePos = visibilityPos + 1;
    const size_t namePos = typePos + 1;
    const size_t argsPos = namePos + 2;
    auto visTokenIt = reduction[visibilityPos].tokens.begin();
    if (visTokenIt != reduction[visibilityPos].tokens.end())
        mVisibility = fromToken(*visTokenIt);
    mRetType = reduction[typePos].tokens;
    mName = reduction[namePos].tokens;
    for (auto arg : reduction[argsPos].nodes)
        arg->dispatch<void>(markVarDecl, VarDecl::argument);
}

std::vector<VarDecl*> Function::args()
{
    return getChildren<VarDecl>();
}

CodeBlock *Function::body()
{
    CodeBlock *res = nullptr;
    meta::walk<CodeBlock, TopDown>(*this, [&res](CodeBlock *node){res = node; return false;}, 1);
    return res;
}

bool Function::is(Function::Attribute attr) const
{
    return (mAttributes & attr) != 0;
}

void Function::set(Function::Attribute attr, bool val)
{
    mAttributes = val ? (mAttributes | attr) : (mAttributes & ~attr);
}

const Declaration::AttributesMap Function::attrMap{
    {"entrypoint", [](Declaration *decl){PRECONDITION(decl->asFunction() != nullptr); decl->asFunction()->set(entrypoint);}}
};

} // namespace meta

