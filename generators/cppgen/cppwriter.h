/*
 * Meta language compiler
 * Copyright (C) 2016  Sergey Vidyuk <sir.vestnik@gmail.com>
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

#include <iostream>

#include "utils/types.h"
#include "utils/contract.h"

#include "typesystem/type.h"

#include "parser/metanodes.h"

namespace meta {
namespace generators {
namespace cppgen {

enum class OutType {
    cpp,
    header
};


enum class Include {
    local,
    global
};

class CppWriter {
    constexpr static char endl = '\n';
public:
    CppWriter(std::ostream& out, OutType type): mOut(&out) {
        if (type == OutType::header)
            out << "#pragma once" << endl;
    }
    CppWriter(const CppWriter&) = delete;
    CppWriter& operator= (const CppWriter&) = delete;
    CppWriter(CppWriter&&) = default;
    CppWriter& operator= (CppWriter&&) = default;

    ~CppWriter() {
        for (; !mPackage.empty(); mPackage = parent(mPackage))
            (*mOut) << '}' << endl;
    }

    void include(const utils::fs::path& path, Include type = Include::local) {
        switch (type) {
        case Include::local: (*mOut) << "#include \"" << path.string() << '"' << endl; break;
        case Include::global: (*mOut) << "#include <" << path.string() << '>' << endl; break;
        }
    }

    void setPackage(const utils::string_view& pkg) {
        for (; !mPackage.empty() && !isSubpackage(pkg, mPackage); mPackage = parent(mPackage))
            (*mOut) << '}' << endl;

        for (auto relativePkg = stripParent(pkg, mPackage); !relativePkg.empty(); ) {
            utils::string_view head;
            std::tie(head, relativePkg) = split(relativePkg, '.');
            assert(!head.empty());
            (*mOut) << "namespace " << head << " {" << endl;
        }
        mPackage = pkg;
    }

    void forwardDeclare(Function* func) {
        PRECONDITION(func->package() == mPackage);
        (*mOut) << func->type()->name() << " " << func->name() << "(";
        bool first = true;
        for (auto* arg: func->args()) {
            if (!first)
                (*mOut) << ",";
            else
                first = false;
            (*mOut) << arg->type()->name() << " " << arg->name();
        }
        (*mOut) << ");" << endl;
    }

private:
    bool isSubpackage(const utils::string_view& child, const utils::string_view& parent) {
        if (child.size() < parent.size())
            return false;
        if (child.size() == parent.size())
            return child == parent;
        return child.substr(0, parent.size()) == parent && child[parent.size()] == '.';
    }

    utils::string_view parent(const utils::string_view& package) {
        auto pos = package.rfind('.');
        if (pos == utils::string_view::npos)
            return {};
        return package.substr(0, pos);
    }

    std::tuple<utils::string_view, utils::string_view> split(const utils::string_view& string, char sep) {
        const auto pos = string.find(sep);
        if (pos == utils::string_view::npos)
            return std::make_tuple(string, utils::string_view{});
        if (pos == 0)
            return std::make_tuple(utils::string_view{}, string);
        return std::make_tuple(string.substr(0, pos), string.substr(pos + 1));
    }

    utils::string_view stripParent(const utils::string_view& package, const utils::string_view& parent) {
        PRECONDITION(parent.empty() || isSubpackage(package, parent));
        if (parent.empty())
            return package;
        if (package.size() == parent.size())
            return {};
        return package.substr(parent.size() + 1);
    }

private:
    utils::string_view mPackage;
    std::ostream* mOut;
};

}}} // namespace meta::generators;:cppgen

