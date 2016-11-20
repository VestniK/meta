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
#include "utils/contract.h"
#include "utils/exception.h"

#include "typesystem/type.h"
#include "typesystem/typesstore.h"

namespace meta::typesystem {

namespace {

class InvalidTypeId: public utils::Exception {
public:
    InvalidTypeId(): utils::Exception(utils::captureBacktrace()) {}

    const char* what() const noexcept override {return "Invalid type id";}
};

class PrimitiveType: public Type {
public:
    explicit PrimitiveType(Type::TypeId id): id(id) {}

    utils::string_view name() const override {
        utils::string_view res;
        POSTCONDITION(!res.empty());
        switch (id) {
        case Auto: res = "auto"; break;
        case Void: res = "void"; break;
        case Int: res = "int"; break;
        case Bool: res = "bool"; break;
        case String: res = "string"; break;

        case UserDefined: throw InvalidTypeId{}; /// @todo think of better enum for id member
        }
        return res;
    }

    TypeId typeId() const override {return id;}

    TypeProps properties() const override {
        switch (id) {
        case Void: return TypeProp::complete | TypeProp::primitive | TypeProp::voidtype;
        case Int: return TypeProp::complete | TypeProp::primitive | TypeProp::numeric;
        case Bool: return TypeProp::complete | TypeProp::primitive | TypeProp::boolean;
        case String: return TypeProp::complete | TypeProp::primitive | TypeProp::sret;
        case Auto: break;

        case UserDefined: throw InvalidTypeId{}; /// @todo think of better enum for id member
        }
        return {};
    }

private:
    TypeId id;
};

} // anonymous namespace

TypesStore::TypesStore(TypesStore* parent): mParent(parent) {
    for (auto primitive: {Type::Auto, Type::Void, Type::Int, Type::Bool, Type::String}) {
        auto type = new PrimitiveType(primitive);
        mTypes[type->name()] = std::unique_ptr<Type>(type);
    }
}

Type* TypesStore::get(utils::string_view name) const {
    const auto it = mTypes.find(name);
    if (it == mTypes.end())
        return mParent ? mParent->get(name) : nullptr;
    return it->second.get();
}

} // namespace meta::typesystem
