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

#include <map>
#include <set>

#include "utils/types.h"

#include "parser/function.h"
#include "parser/struct.h"

namespace meta {

template<typename Decl>
struct NameComparator {
    using is_transparent = void;

    bool operator() (const Decl* lhs, const Decl* rhs) const {
        return lhs->name() < rhs->name();
    }

    bool operator() (const Decl& lhs, const Decl& rhs) const {
        return lhs.name() < rhs.name();
    }

    bool operator() (const Decl* lhs, utils::string_view rhs) const {
        return lhs->name() < rhs;
    }

    bool operator() (const Decl& lhs, utils::string_view rhs) const {
        return lhs.name() < rhs;
    }

    bool operator() (utils::string_view lhs, const Decl* rhs) const {
        return lhs < rhs->name();
    }

    bool operator() (utils::string_view lhs, const Decl& rhs) const {
        return lhs < rhs.name();
    }
};

template<typename T>
using Dict = std::set<T, NameComparator<typename std::remove_pointer<T>::type>>;
template<typename T>
using MultiDict = std::multiset<T, NameComparator<typename std::remove_pointer<T>::type>>;

struct PackageDict {
    MultiDict<Function*> functions;
    Dict<Struct*> structs;
};

using Dictionary = std::map<utils::string_view, PackageDict>;

} // namespace meta
