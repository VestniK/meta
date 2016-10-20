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

#include "utils/array_view.h"
#include "utils/range.h"

#include "parser/dictionary.h"
#include "parser/metaparser.h"
#include "parser/metanodes.h"
#include "parser/visibility.h"

#include "analysers/declconflicts.h"
#include "analysers/resolver.h"
#include "analysers/semanticerror.h"

namespace meta::analysers {
namespace {

void trace([[gnu::unused]]Node* node) {
//     std::clog << node->sourceLocation() << ':' << node->tokens().linenum() << ':' << node->tokens().colnum();
//     utils::string_view str = node->tokens();
//     str = str.substr(0, str.find('\n'));
//     std::clog << ": " << str << '\n';
}

bool isChildPackage(utils::string_view subpkg, utils::string_view parentpkg) {
    if (parentpkg.length() > subpkg.length())
        return false;
    if (subpkg[parentpkg.size()] != '.')
        return false;
    const auto part = subpkg.substr(0, parentpkg.size());
    return part == parentpkg;
}

template<typename Decl>
struct DeclRef {
    Decl* decl;
    Import* import = nullptr;

    utils::string_view name() const {
        if (import)
            return import->name();
        return decl->name();
    }
};

struct VarStats {
    VarStats(VarDecl* decl):
        decl(decl),
        assignCount((decl->flags() & VarFlags::argument) || decl->inited() ? 1 : 0)
    {}

    VarDecl* decl;
    unsigned assignCount;
    unsigned accessCount = 0;

    utils::string_view name() const {return decl->name();}
};

template<typename Decl>
Decl* decl(const DeclRef<Decl>& val) {return val.decl;}

template<typename Decl>
Import* import(const DeclRef<Decl>& val) {return val.import;}

struct CodeContext {
    CodeContext* parent = nullptr;
    utils::string_view package;
    MultiDict<DeclRef<Function>> functions;
    Dict<DeclRef<Struct>> structs;
    std::map<utils::string_view, VarStats> vars;

    ~CodeContext() noexcept(false) {
        if (std::uncaught_exceptions() != 0)
            return;
        for (const auto& kv: vars) {
            if (kv.second.accessCount == 0)
                throw SemanticError(kv.second.decl, "Variable '%s' is never used", kv.first);
        }
    }
};

template<typename Decl>
Decl* find(CodeContext& ctx, utils::string_view name);

template<>
VarStats* find<VarStats>(CodeContext& ctx, utils::string_view name) {
    for (auto* context = &ctx; context != nullptr; context = context->parent) {
        auto it = context->vars.find(name);
        if (it != context->vars.end())
            return &(it->second);
    }
    return nullptr;
}

struct Resolver {
    Dictionary& dict;

    void operator() (Node* node, CodeContext&) {
        trace(node);
        throw UnexpectedNode(node, "Don't know how to resolve declaration and types for");
    }

    void operator() (SourceFile* node, CodeContext& ctx) {
        trace(node);
        CodeContext srcFileContext = {&ctx, node->package()};
        for (auto* func: dict[node->package()].functions)
            srcFileContext.functions.emplace(DeclRef<Function>{func});
        for (auto* strct: dict[node->package()].structs)
            srcFileContext.structs.emplace(DeclRef<Struct>{strct});

        /// @todo split SourceFile children into imports, functions and structs
        for (auto import: node->getChildren<Import>())
            (*this)(import, srcFileContext);

        for (auto structure: node->getChildren<Struct>())
            (*this)(structure, srcFileContext);

        for (auto func: node->getChildren<Function>())
            (*this)(func, srcFileContext);
    }

    void operator() (Import* node, CodeContext& ctx) {
        trace(node);
        POSTCONDITION(!node->importedDeclarations().empty());
        POSTCONDITION(
            node->importedDeclarations().size() == 1 ||
            utils::count_if(node->importedDeclarations(), [](Declaration* decl) {
                return decl->getVisitableType() != std::type_index(typeid(Function));
            }) == 0
        );
        if (node->targetPackage() == ctx.package)
            throw SemanticError(node, "Import of a declaration from the current package is meaningless");
        auto pkgIt = dict.find(node->targetPackage());
        if (pkgIt == dict.end())
            throw SemanticError(node, "No such package '%s'", node->targetPackage());
        auto structIt = pkgIt->second.structs.find(node->target());
        auto funcs = utils::equal_range(pkgIt->second.functions, node->target());
        if (structIt != pkgIt->second.structs.end()) {
            if ((*structIt)->visibility() == Visibility::Private)
                throw SemanticError(node, "Struct '%s' is private in the package '%s'", node->name(), node->targetPackage());
            if (
                (*structIt)->visibility() == Visibility::Protected &&
                !isChildPackage(ctx.package, node->targetPackage())
            ) {
                throw SemanticError(
                    node, "Struct '%s' is protected in the package '%s' which is not parent of current package '%s'",
                    node->name(), node->targetPackage(), ctx.package
                );
            }
            const auto conflictingFuncs = utils::equal_range(ctx.functions, node->name());
            if (!conflictingFuncs.empty())
                throwDeclConflict(node, conflictingFuncs);
            const auto insres = ctx.structs.emplace(DeclRef<Struct>{*structIt, node});
            if (!insres.second)
                throwDeclConflict(node, utils::slice(insres.first));
            node->addImportedDeclaration(*structIt);
        } else if (!funcs.empty()) {
            auto conflictingStruct = ctx.structs.find(node->name());
            if (conflictingStruct != ctx.structs.end())
                throwDeclConflict(node, utils::slice(conflictingStruct));
            for (auto* func: funcs) {
                if (func->visibility() == Visibility::Private)
                    continue;
                if (
                    func->visibility() == Visibility::Protected &&
                    !isChildPackage(ctx.package, node->targetPackage())
                )
                    continue;
                ctx.functions.emplace(DeclRef<Function>{func, node});
                node->addImportedDeclaration(func);
            }
            if (node->importedDeclarations().empty()) {
                std::ostringstream oss;
                oss <<
                    "Function '" << node->name() << "' from the package '" << node->targetPackage() <<
                    "' has no overloads visible from the current package '" << ctx.package << '\''
                ;
                for (const auto& func: funcs) {
                    oss <<
                        "\n" << SourceInfo{func} << ": notice: " << declinfo(func) <<
                        " is " << func->visibility()
                    ;
                }
                throw SemanticError(node, "%s", oss.str());
            }
        } else {
            throw SemanticError(
                node, "Declaration '%s' not found in package '%s'",
                node->target(), node->targetPackage()
            );
        }
    }

    void operator() (Function* node, CodeContext& ctx) {
        trace(node);
        if (node->visibility() != Visibility::Extern && node->body() == nullptr)
            throw SemanticError(node, "Implementation missing for the function '%s'", node->name());
        if (node->visibility() == Visibility::Extern && node->body() != nullptr)
            throw SemanticError(node, "Extern function '%s' must not have implementation", node->name());

        const auto it = utils::adjacent_find(node->args(), [](VarDecl* prev, VarDecl* next) {
            return prev->inited() && !next->inited();
        });
        if (it != node->args().end()) {
            throw SemanticError(
                *it,
                "Argument '%s' has no default value while previous argument '%s' has",
                (*(it + 1))->name(), (*it)->name()
            );
        }

        if (!node->body())
            return;
        CodeContext funcContext{&ctx};
        for (auto arg: node->args()) {
            auto res = funcContext.vars.emplace(arg->name(), VarStats{arg});
            if (!res.second)
                throw SemanticError(
                    arg, "%s has more than one arguments with the same name '%s'",
                    declinfo(node), arg->name()
                );
        }
        for (auto statement: node->body()->statements())
            dispatch(*this, statement, funcContext);
    }

    void operator() (Call* node, CodeContext& ctx) {
        trace(node);
        POSTCONDITION(node->function() != nullptr);
        for (auto* currctx = &ctx; currctx != nullptr; currctx = currctx->parent) {
            auto matches = utils::equal_range(currctx->functions, node->functionName());
            if (matches.empty())
                continue;
            /// @todo replace assert by proper support of function overload
            assert(std::distance(matches.begin(), matches.end()) == 1);
            node->setFunction(matches.begin()->decl);
            auto expectedArgs = node->function()->args();
            auto passedArgs = node->args();
            if (expectedArgs.size() != passedArgs.size()) {
                throw SemanticError(
                    node,
                    "%s is called with incorrect number of arguments",
                    declinfo(node->function())
                );
            }
            break;
        }
        if (!node->function())
            throw SemanticError(node, "Unresolved function call '%s'", node->functionName());
        for (Expression* arg: node->args())
            dispatch(*this, arg, ctx);
    }

    void operator() (CodeBlock* node, CodeContext& ctx) {
        trace(node);
        CodeContext blockCtx{ctx};
        for (auto statement: node->statements())
            dispatch(*this, statement, blockCtx);
    }

    void operator() (VarDecl* node, CodeContext& ctx) {
        trace(node);
        auto conflict = find<VarStats>(ctx, node->name());
        if (conflict && (conflict->decl->flags() & VarFlags::argument))
            throwDeclConflict(node, conflict->decl);
        if (node->inited() && !(node->flags() & VarFlags::argument))
            dispatch(*this, node->initExpr(), ctx);
        auto res = ctx.vars.emplace(node->name(), VarStats{node});
        if (!res.second)
            throwDeclConflict(node, res.first->second.decl);
    }

    void operator() (If* node, CodeContext& ctx) {
        trace(node);
        dispatch(*this, node->condition(), ctx);
        if (node->thenBlock()) {
            CodeContext thenCtx{ctx};
            dispatch(*this, node->thenBlock(), thenCtx);
        }
        if (node->elseBlock()) {
            CodeContext elseCtx{ctx};
            dispatch(*this, node->elseBlock(), elseCtx);
        }
    }

    void operator() (BinaryOp* node, CodeContext& ctx) {
        trace(node);
        dispatch(*this, node->left(), ctx);
        dispatch(*this, node->right(), ctx);
    }

    void operator() (PrefixOp* node, CodeContext& ctx) {
        trace(node);
        dispatch(*this, node->operand(), ctx);
    }

    void operator() (Number* node, CodeContext&) {trace(node);}
    void operator() (StrLiteral* node, CodeContext&) {trace(node);}
    void operator() (Literal* node, CodeContext&) {trace(node);}

    void operator() (Var* node, CodeContext& ctx) {
        trace(node);
        auto varstat = find<VarStats>(ctx, node->name());
        if (!varstat)
            throw SemanticError(node, "Undefined variable '%s'", node->name());
        if (varstat->assignCount == 0)
            throw SemanticError(node, "Variable '%s' accessed before initialization", node->name());
        varstat->accessCount++;
        node->setDeclaration(varstat->decl);
    }

    void operator() (Assigment* node, CodeContext& ctx) {
        trace(node);
        if (node->target()->getVisitableType() == std::type_index(typeid(Var))) {
            auto target = static_cast<Var*>(node->target());
            auto stats = find<VarStats>(ctx, target->name());
            if (stats->decl->flags() & VarFlags::argument)
                throw SemanticError(node, "Attempt to modify function argument '%s'", target->name());
            stats->assignCount++;
            target->setDeclaration(stats->decl);
        } else if (node->target()->getVisitableType() == std::type_index(typeid(MemberAccess))) {
            throw UnexpectedNode(node->target(), "Member assigment is not yet implemented");
        } else
            throw UnexpectedNode(node->target(), "Unexpected assigment left side expression type");
        dispatch(*this, node->value(), ctx);
    }

    void operator() (Return* node, CodeContext& ctx) {
        trace(node);
        if (node->value())
            dispatch(*this, node->value(), ctx);
    }

    void operator() (Struct* node, CodeContext&) {
        trace(node);
    }
};

}

class ResolveVisitor: public Visitor {
public:
    ResolveVisitor(Dictionary &dict): mGlobalDict(dict)
    {
    }

    virtual bool visit(SourceFile *node) override
    {
        mCurrDecls.clear();
        mCurrDecls = mGlobalDict[node->package()].functions;
        mCurrSrcPackage = node->package();
        return true;
    }

    virtual void leave(Import* node) override
    {
        if (node->targetPackage() == mCurrSrcPackage)
            return;
        for (Function *func: utils::equal_range(mGlobalDict[node->targetPackage()].functions, node->target())) {
            if (func->visibility() == Visibility::Private)
                throw SemanticError(node, "Can't import private declaration '%s.%s'", func->package(), func->name());
            if (func->visibility() == Visibility::Protected && !isChildPackage(mCurrSrcPackage, node->targetPackage()))
                throw SemanticError(
                    node, "Can't import protected declaration '%s.%s' from package '%s' which is not a subpackage of '%s'",
                    func->package(), func->name(), mCurrSrcPackage, node->targetPackage()
                );
            assert(mCurrDecls.count(node->name()) == 0); /// @todo support for function overloads instead of assert here!!!
            mCurrDecls.insert(func);
        }
    }

    virtual bool visit(Call *node) override
    {
        assert(mCurrDecls.count(node->functionName()) == 1); /// @todo support for function overloads instead of assert here!!!
        auto it = mCurrDecls.lower_bound(node->functionName());
        if (it == mCurrDecls.end())
            throw SemanticError(node, "Unresolved function call '%s'", node->functionName());
        node->setFunction(*it); /// @todo: handle overloads
        auto expectedArgs = node->function()->args();
        auto passedArgs = node->getChildren<Node>(1);
        if (expectedArgs.size() != passedArgs.size())
            throw SemanticError(node, "Call to function '%s' with incorrect number of arguments", node->functionName());
        return true;
    }

    virtual bool visit(Function *node) override
    {
        if (node->visibility() != Visibility::Extern && node->body() == nullptr)
            throw SemanticError(node, "Implementation missing for the function '%s'", node->name());
        if (node->visibility() == Visibility::Extern && node->body() != nullptr)
            throw SemanticError(node, "Extern function '%s' must not have implementation", node->name());

        bool hasDefaultValues = false;
        for (auto arg : node->args()) {
            if (arg->inited())
                hasDefaultValues = true;
            if (hasDefaultValues && !arg->inited())
                throw SemanticError(arg, "Argument '%s' has no default value while previous argument has", arg->name());
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
                throw SemanticError(var.second.decl, "Variable '%s' declared but never used", var.first);
        }
    }

    virtual bool visit(VarDecl *node) override
    {
        auto prev = mVars.find(node->name());
        if (prev != mVars.end())
            throw SemanticError(
                node, "Redefinition of the variable '%s' first defined at line %d, column %d",
                node->name(),
                prev->second.decl->tokens().begin()->line, prev->second.decl->tokens().begin()->column
            );
        mVars[node->name()] = VarSrc(node);
        return true;
    }

    virtual bool visit(Var *node) override
    {
        auto decl = mVars.find(node->name());
        if (decl == mVars.end())
            throw SemanticError(node, "Reference to undefined variable '%s'", node->name());
        if (decl->second.modifyCount == 0)
            throw SemanticError(node, "Variable '%s' used before initialization", node->name());
        node->setDeclaration(decl->second.decl);
        ++decl->second.accessCount;
        return true;
    }

    bool visit(Assigment *node) override {
        if (Var* var = dynamic_cast<Var*>(node->target())) {
            auto decl = mVars.find(var->name());
            ++decl->second.modifyCount;
            if (decl->second.decl->flags() & VarFlags::argument)
                throw SemanticError(node, "Attempt to modify function argument '%s'", var->name());
        } else if (dynamic_cast<MemberAccess*>(node->target()))
            throw UnexpectedNode(node->target(), "Assign value to struct member is not yet implemented");
        else
            throw UnexpectedNode(node->target(), "Incorrect assign target");
        return true;
    }

private:
    struct VarSrc {
        VarSrc(VarDecl *decl = nullptr):
            decl(decl),
            modifyCount(decl && (decl->inited() || (decl->flags() & VarFlags::argument)) ? 1 : 0),
            accessCount(0)
        {}

        VarDecl *decl;
        unsigned modifyCount;
        unsigned accessCount;
    };

    Dictionary& mGlobalDict;
    MultiDict<Function*> mCurrDecls;
    utils::string_view mCurrSrcPackage;
    std::map<utils::string_view, VarSrc> mVars;
};

inline namespace v1 {

void resolve(AST* ast, Dictionary& dict)
{
    ResolveVisitor resolver(dict);
    ast->walk(&resolver);
}

} // namespace v1

namespace v2 {

void resolve(AST* ast, Dictionary& dict) {
    Resolver resolver{dict};
    CodeContext globalCtx;
    for (auto root: ast->getChildren<Node>(0))
        dispatch(resolver, root, globalCtx);
}

} // namespace v2

} // namespace meta::analysers
