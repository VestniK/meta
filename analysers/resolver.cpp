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
 */
#include <cassert>
#include <map>
#include <set>
#include <algorithm>

#include "parser/actions.h"
#include "parser/assigment.h"
#include "parser/call.h"
#include "parser/function.h"
#include "parser/import.h"
#include "parser/metaparser.h"
#include "parser/sourcefile.h"
#include "parser/var.h"
#include "parser/vardecl.h"

#include "analysers/resolver.h"
#include "analysers/semanticerror.h"

namespace meta {
namespace analysers {

namespace
{

inline
bool isChildPackage(const std::experimental::string_view& subpkg, const std::experimental::string_view& parentpkg)
{
    /// @todo rewrite using TokenSequence
    if (parentpkg.length() > subpkg.length())
        return false;
    size_t pos = 0;
    for (; pos < parentpkg.length(); ++pos) {
        if (subpkg[pos] != parentpkg[pos])
            return false;
    }
    return subpkg[pos] == '.';
}

}

class ResolveVisitor: public Visitor
{
public:
    ResolveVisitor(Dictionary &dict): mGlobalDict(dict)
    {
    }

    virtual bool visit(SourceFile *node) override
    {
        mCurrDecls.clear();
        mCurrDecls = mGlobalDict[node->package()];
        mCurrSrcPackage = node->package();
        return true;
    }

    virtual void leave(Import *node) override
    {
        if (node->targetPackage() == mCurrSrcPackage)
            return;
        for (
            auto it = mGlobalDict[node->targetPackage()].lower_bound(node->target());
            it != mGlobalDict[node->targetPackage()].upper_bound(node->target());
            ++it
        ) {
            Function *func = it->second;
            if (func->visibility() == Visibility::Private)
                throw SemanticError(
                    node, "Can't import private declaration '%*.s.%*.s'",
                    static_cast<int>(func->package().size()), func->package().data(),
                    static_cast<int>(func->name().size()), func->name().data()
                );
            if (func->visibility() == Visibility::Protected && !isChildPackage(mCurrSrcPackage, node->targetPackage()))
                throw SemanticError(
                    node, "Can't import protected declaration '%*.s.%*.s' from package '%*.s' which is not a subpackage of '%*.s'",
                    static_cast<int>(func->package().size()), func->package().data(),
                    static_cast<int>(func->name().size()), func->name().data(),
                    static_cast<int>(mCurrSrcPackage.size()), mCurrSrcPackage.data(),
                    static_cast<int>(node->targetPackage().size()), node->targetPackage().data()
                );
            assert(mCurrDecls.count(node->name()) == 0); /// @todo support for function overloads instead of assert here!!!
            mCurrDecls.insert({node->name(), it->second});
        }
    }

    virtual bool visit(Call *node) override
    {
        assert(mCurrDecls.count(node->functionName()) == 1); /// @todo support for function overloads instead of assert here!!!
        auto it = mCurrDecls.lower_bound(node->functionName());
        if (it == mCurrDecls.end())
            throw SemanticError(
                node, "Unresolved function call '%*.s'",
                static_cast<int>(node->functionName().size()), node->functionName().data()
            );
        node->setFunction(it->second); /// @todo: handle overloads
        auto expectedArgs = node->function()->args();
        auto passedArgs = node->getChildren<Node>(1);
        if (expectedArgs.size() != passedArgs.size())
            throw SemanticError(
                node, "Call to function '%*.s' with incorrect number of arguments",
                static_cast<int>(node->functionName().size()), node->functionName().data()
            );
        return true;
    }

    virtual bool visit(Function *node) override
    {
        if (node->visibility() != Visibility::Extern && node->body() == nullptr)
            throw SemanticError(
                node, "Implementation missing for the function '%*.s'",
                static_cast<int>(node->name().size()), node->name().data()
            );
        if (node->visibility() == Visibility::Extern && node->body() != nullptr)
            throw SemanticError(
                node, "Extern function '%*.s' must not have implementation",
                static_cast<int>(node->name().size()), node->name().data()
            );

        bool hasDefaultValues = false;
        for (auto arg : node->args()) {
            if (arg->inited())
                hasDefaultValues = true;
            if (hasDefaultValues && !arg->inited())
                throw SemanticError(
                    arg, "Argument '%*.s' has no default value while previous argument has",
                    static_cast<int>(arg->name().size()), arg->name().data()
                );
        }
        mVars.clear();
        return true;
    }

    virtual void leave(Function *node) override
    {
        if (node->visibility() == Visibility::Extern)
            return; // Skip check for unused arguments on extern functions since they have no implementation to use them
        for (const auto &var : mVars) {
            if (var.second.accessCount == 0)
                throw SemanticError(
                    var.second.decl, "Variable '%*.s' declared but never used",
                    static_cast<int>(var.first.size()), var.first.data()
                );
        }
    }

    virtual bool visit(VarDecl *node) override
    {
        auto prev = mVars.find(node->name());
        if (prev != mVars.end())
            throw SemanticError(
                node, "Redefinition of the variable '%*.s' first defined at line %d, column %d",
                static_cast<int>(node->name().size()), node->name().data(),
                prev->second.decl->tokens().begin()->line, prev->second.decl->tokens().begin()->column
            );
        mVars[node->name()] = VarSrc(node);
        return true;
    }

    virtual bool visit(Var *node) override
    {
        auto decl = mVars.find(node->name());
        if (decl == mVars.end())
            throw SemanticError(
                node, "Reference to undefined variable '%*.s'",
                static_cast<int>(node->name().size()), node->name().data()
            );
        if (decl->second.modifyCount == 0)
            throw SemanticError(
                node, "Variable '%*.s' used before initialization",
                static_cast<int>(node->name().size()), node->name().data()
            );
        node->setDeclaration(decl->second.decl);
        ++decl->second.accessCount;
        return true;
    }

    virtual bool visit(Assigment *node) override
    {
        auto decl = mVars.find(node->varName());
        if (decl == mVars.end())
            throw SemanticError(
                node, "Reference to undefined variable '%*.s'",
                static_cast<int>(node->varName().size()), node->varName().data()
            );
        if (decl->second.decl->is(VarDecl::argument))
            throw SemanticError(
                node, "Attempt to modify function argument '%*.s'",
                static_cast<int>(node->varName().size()), node->varName().data()
            );
        node->setDeclaration(decl->second.decl);
        ++decl->second.modifyCount;
        return true;
    }

private:
    struct VarSrc
    {
        VarSrc(VarDecl *decl = nullptr):
            decl(decl),
            modifyCount(decl && (decl->inited() || decl->is(VarDecl::argument)) ? 1 : 0),
            accessCount(0)
        {}

        VarDecl *decl;
        unsigned modifyCount;
        unsigned accessCount;
    };

    class FuncComparator
    {
    public:
        bool operator() (Function *left, Function *right) const
        {
            if (left->package().compare(right->package()) < 0)
                return true;
            if (left->name().compare(right->name()) < 0)
                return true;
            return false;
        }
    };

    Dictionary& mGlobalDict;
    DeclarationsDict mCurrDecls;
    std::experimental::string_view mCurrSrcPackage;
    std::map<std::experimental::string_view, VarSrc> mVars;
};

void resolve(AST* ast, Dictionary& dict)
{
    ResolveVisitor resolver(dict);
    ast->walk(&resolver);
}

} // namespace analysers
} // namespace meta

