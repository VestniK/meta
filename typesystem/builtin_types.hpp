#pragma once

#include "utils/contract.h"
#include "utils/exception.h"

#include "typesystem/type.h"

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

std::vector<std::unique_ptr<Type>> createBuiltinTypes() {
    std::vector<std::unique_ptr<Type>> res;
    for (auto primitive: {Type::Auto, Type::Void, Type::Int, Type::Bool, Type::String})
        res.emplace_back(new PrimitiveType(primitive));
    return res;
}

} // namespace meta::typesystem
