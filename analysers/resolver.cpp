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

#include <map>

#include "parser/assigment.h"
#include "parser/call.h"
#include "parser/function.h"
#include "parser/metaparser.h"
#include "parser/package.h"
#include "parser/var.h"
#include "parser/vardecl.h"

#include "analysers/resolver.h"
#include "analysers/semanticerror.h"

namespace analysers {

class ResolveVisitor: public meta::Visitor
{
public:
    ResolveVisitor(meta::AST *ast)
    {
        for (auto pkg : ast->getChildren<meta::Package>()) {
            auto pkgFuncs = pkg->functions();
            mFunctions.insert(mFunctions.end(), pkgFuncs.begin(), pkgFuncs.end());
        }
    }

    virtual void visit(meta::Call *node) override
    {
        for (const auto func : mFunctions) {
            if (node->functionName() != func->name())
                continue;
            node->setFunction(func);
            break;
        }
        if (node->function() == nullptr)
            throw SemanticError(node, "Unresolved function call '%s'", node->functionName().c_str());
        auto expectedArgs = node->function()->args();
        auto passedArgs = node->getChildren<meta::Node>(1);
        if (expectedArgs.size() != passedArgs.size())
            throw SemanticError(node, "Call to function '%s' with incorrect number of arguments", node->functionName().c_str());
        /// @todo check types
    }

    virtual void visit(meta::Function *) override
    {
        mVars.clear();
    }

    virtual void leave(meta::Function *) override
    {
        for (const auto &var : mVars) {
            if (var.second.accessCount == 0)
                throw SemanticError(var.second.decl, "Variable '%s' declared but never used", var.first.c_str());
        }
    }

    virtual void visit(meta::VarDecl *node) override
    {
        auto prev = mVars.find(node->name());
        if (prev != mVars.end())
            throw SemanticError(
                node, "Redefinition of the variable '%s' first defined at line %d, column %d",
                node->name().c_str(), prev->second.decl->tokens().begin()->line, prev->second.decl->tokens().begin()->column
            );
        mVars[node->name()] = VarSrc(node);
    }

    virtual void visit(meta::Var *node) override
    {
        auto decl = mVars.find(node->name());
        if (decl == mVars.end())
            throw SemanticError(node, "Reference to undefined variable '%s'", node->name().c_str());
        if (decl->second.modifyCount == 0)
            throw SemanticError(node, "Variable '%s' used before initialization", node->name().c_str());
        node->setDeclaration(decl->second.decl);
        ++decl->second.accessCount;
    }

    virtual void visit(meta::Assigment *node) override
    {
        auto decl = mVars.find(node->varName());
        if (decl == mVars.end())
            throw SemanticError(node, "Reference to undefined variable '%s'", node->varName().c_str());
        if (decl->second.decl->is(meta::VarDecl::argument))
            throw SemanticError(node, "Attempt to modify function argument '%s'", node->varName().c_str());
        node->setDeclaration(decl->second.decl);
        ++decl->second.modifyCount;
    }

private:
    struct VarSrc
    {
        VarSrc(meta::VarDecl *decl = nullptr):
            decl(decl),
            modifyCount(decl && (decl->inited() || decl->is(meta::VarDecl::argument)) ? 1 : 0),
            accessCount(0)
        {}

        meta::VarDecl *decl;
        unsigned modifyCount;
        unsigned accessCount;
    };

    std::vector<meta::Function*> mFunctions;
    std::map<std::string, VarSrc> mVars;
};

void resolve(meta::AST *ast)
{
    ResolveVisitor resolver(ast);
    ast->walk(&resolver);
}

} // namespace analysers
