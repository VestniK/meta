#pragma once

#include <map>

#include "utils/types.h"

#include "typesystem/type.h"

#include "parser/metanodes.h"

#include "analysers/dictionary.h"

namespace meta::analysers {
namespace {

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

using Type = typesystem::Type;

struct Scope {
    Scope* parent = nullptr;
    utils::string_view package;
    utils::multidict<DeclRef<Function>> functions;
    utils::dict<DeclRef<Struct>> structs;
    std::map<utils::string_view, VarStats> vars;
    utils::dict<typesystem::Type> types;

    /// Создание глобального контекста
    Scope() {
        utils::copy(typesystem::builtinTypes(), std::inserter(types, types.end()));
    }
    Scope(Scope* parent, utils::string_view package = {}): parent(parent), package(package) {}

    Scope(const Scope&) = delete;
    Scope(Scope&&) = delete;
    const Scope& operator= (const Scope&) = delete;
    Scope& operator= (Scope&&) = delete;

    ~Scope() noexcept(false) {
        if (std::uncaught_exceptions() != 0)
            return;
        for (const auto& kv: vars) {
            if (kv.second.accessCount == 0)
                throw SemanticError(kv.second.decl, "Variable '%s' is never used", kv.first);
        }
    }

    template<typename Decl>
    Decl* find(utils::string_view name) = delete;

    utils::optional<typesystem::Type> findType(utils::string_view name) const {
        for (auto* scope = this; scope != nullptr; scope = scope->parent) {
            auto it = scope->types.find(name);
            if (it != scope->types.end())
                return *it;
        }
        return utils::nullopt;
    }
};

template<>
VarStats* Scope::find<VarStats>(utils::string_view name) {
    for (auto* scope = this; scope != nullptr; scope = scope->parent) {
        auto it = scope->vars.find(name);
        if (it != scope->vars.end())
            return &(it->second);
    }
    return nullptr;
}

} // anonymous namespace
} // namespace meta::analysers
