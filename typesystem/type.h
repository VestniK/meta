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

#include <memory>
#include <vector>

#include "utils/bitmask.h"
#include "utils/types.h"

namespace meta::typesystem {

namespace BuiltinType {

constexpr static utils::string_view Auto = "auto"sv;
constexpr static utils::string_view Int = "int"sv;
constexpr static utils::string_view Bool = "bool"sv;
constexpr static utils::string_view String = "string"sv;
constexpr static utils::string_view Void = "void"sv;
constexpr static utils::string_view Double = "double"sv;

};

enum class TypeProp {
    complete,
    voidtype,
    numeric,
    boolean,
    primitive,
    tuple,
    sum,
    array,
    namedComponents,
    sret
};
using TypeProps = utils::Bitmask<TypeProp>;
constexpr
TypeProps operator| (TypeProp lhs, TypeProp rhs) {
    return TypeProps{lhs} | rhs;
}

class Type {
public:
    enum TypeId {
        // incomplete types
        Auto,

        // Built in types
        Void,
        Int,
        Bool,
        Double,
        String
    };

    constexpr Type(TypeId id): mId(id) {}

    utils::string_view name() const;
    TypeId typeId() const {return mId;}
    TypeProps properties() const;

    bool operator== (Type rhs) const {return mId == rhs.mId;}
    bool operator!= (Type rhs) const {return mId != rhs.mId;}

private:
    TypeId mId;
};

utils::array_view<Type> builtinTypes();

} // namespace meta::typesystem
