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

#include <experimental/string_view>

#include "utils/contract.h"

#include "typesystem/type.h"
#include "typesystem/typesstore.h"

namespace meta {
namespace typesystem {

namespace {

class PrimitiveType: public Type
{
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
        }
        return res;
    }

    virtual TypeId typeId() const {return id;}

private:
    TypeId id;
};

} // anonymous namespace

TypesStore::TypesStore()
{
    Type::TypeId primitives[] = {
        Type::Auto, Type::Void, Type::Int, Type::Bool, Type::String
    };
    for (auto primitive : primitives) {
        auto type = new PrimitiveType(primitive);
        mTypes[type->name()] = std::unique_ptr<Type>(type);
    }
}

TypesStore::~TypesStore()
{
}

Type* TypesStore::getByName(const utils::string_view& name) const
{
    auto it = mTypes.find(name);
    if (it == mTypes.end())
        return nullptr;
    return it->second.get();
}

Type *TypesStore::getPrimitive(Type::TypeId id) const
{
    if ((id & Type::primitive) == 0)
        return nullptr;
    for (const auto &pair : mTypes) {
        if (pair.second->typeId() == id)
            return pair.second.get();
    }
    return nullptr;
}

Type *TypesStore::getVoid() const
{
    auto it = mTypes.find("void");
    assert(it != mTypes.end());
    return it->second.get();
}

} // namespace typesystem
} // namespace meta
