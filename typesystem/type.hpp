#pragma once

#include "utils/contract.h"
#include "utils/exception.h"

#include "typesystem/type.h"

namespace meta::typesystem {

utils::string_view Type::name() const {
    switch (mId) {
    case Auto: return BuiltinType::Auto;
    case Void: return BuiltinType::Void;
    case Int: return BuiltinType::Int;
    case Bool: return BuiltinType::Bool;
    case String: return BuiltinType::String;
    }
    assert(false);
    return {};
}

TypeProps Type::properties() const {
    switch (mId) {
    case Void: return TypeProp::complete | TypeProp::primitive | TypeProp::voidtype;
    case Int: return TypeProp::complete | TypeProp::primitive | TypeProp::numeric;
    case Bool: return TypeProp::complete | TypeProp::primitive | TypeProp::boolean;
    case String: return TypeProp::complete | TypeProp::primitive | TypeProp::sret;
    case Auto: break;
    }
    return {};
}

utils::array_view<Type> builtinTypes() {
    static const Type types[] = {
        Type::Auto,
        Type::Void,
        Type::Int,
        Type::Bool,
        Type::String
    };
    return types;
}

} // namespace meta::typesystem
