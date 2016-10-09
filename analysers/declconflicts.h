#pragma once

#include <iostream>

#include "utils/range.h"

#include "parser/function.h"
#include "parser/import.h"
#include "parser/struct.h"

#include "analysers/semanticerror.h"

namespace meta::analysers {

struct SourceInfo {
    SourceInfo(Node* node):
        location(node->sourceLocation()),
        line(node->tokens().linenum()),
        column(node->tokens().colnum())
    {}
    SourceInfo(const NodeException& err):
        location(err.sourcePath()),
        line(err.tokens().linenum()),
        column(err.tokens().colnum())
    {}

    std::string location;
    int line, column;
};

inline
std::ostream& operator<< (std::ostream& out, const SourceInfo& info) {
    out << info.location << ':' << info.line << ':' << info.column;
    return out;
}

template<typename Decl>
struct Declinfo {
    Decl* decl;
};

template<typename Decl>
auto declinfo(Decl* decl) {return Declinfo<Decl>{decl};}

inline
std::ostream& operator<< (std::ostream& out, const Declinfo<Import>& info) {
    out <<
        "Import of '" << info.decl->targetPackage() << '.' << info.decl->target() <<
        "' as '" << info.decl->name() << '\''
    ;
    return out;
}

inline
std::ostream& operator<< (std::ostream& out, const Declinfo<Struct>& info) {
    out << "Struct '" << info.decl->package() << '.' << info.decl->name() << '\'';
    return out;
}

inline
std::ostream& operator<< (std::ostream& out, const Declinfo<VarDecl>& info) {
    out << "Variable '" << info.decl->typeName() << ' ' << info.decl->name() << '\'';
    if (info.decl->flags() & VarFlags::argument)
        out << " (function argument)";
    return out;
}

inline
std::ostream& operator<< (std::ostream& out, const Declinfo<Function>& info) {
    out << "Function '" << info.decl->package() << '.' << info.decl->name() << '(';
    bool first = true;
    for (const auto& arg: info.decl->args()) {
        if (first)
            first = false;
        else
            out << ", ";
        out << arg->typeName();
    }
    out << ")'";
    return out;
}

inline
Import* decl(Import* node) {return node;}
inline
Struct* decl(Struct* node) {return node;}
inline
Function* decl(Function* node) {return node;}
inline
VarDecl* decl(VarDecl* node) {return node;}

inline
Import* import(Node*) {return nullptr;}

template<typename Decl, typename Range>
[[noreturn]]
void throwDeclConflict(Decl* node, const Range& range) {
    std::ostringstream oss;
    oss << declinfo(node) << " conflicts with other declarations.";
    for (const auto& conflict: range) {
        oss << '\n' << SourceInfo{decl(conflict)} << ": notice: " << declinfo(decl(conflict));
        auto imported = import(conflict);
        if (imported)
            oss << "\n\timported as '" << imported->name() << "' here: " << SourceInfo{imported};
    }
    throw SemanticError(node, "%s", oss.str());
}

template<typename Decl1, typename Decl2>
[[noreturn]]
void throwDeclConflict(Decl1* node, Decl2* conflict) {
    std::ostringstream oss;
    oss << declinfo(node) << " conflicts with other declarations.";
    oss << '\n' << SourceInfo{conflict} << ": notice: " << declinfo(decl(conflict));
    auto imported = import(conflict);
    if (imported)
        oss << "\n\timported as '" << imported->name() << "' here: " << SourceInfo{imported};
    throw SemanticError(node, "%s", oss.str());
}

} // namespace meta::analysers
