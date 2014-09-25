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

#ifndef TYPESSTORE_H
#define TYPESSTORE_H

#include <map>
#include <memory>
#include <string>

#include "typesystem/type.h"

namespace typesystem {

class TypesStore
{
public:
    TypesStore();
    ~TypesStore();

    Type *getByName(const std::string &name) const;
    Type *getPrimitive(Type::TypeId id) const;

private:
    std::map< std::string, std::unique_ptr<Type> > mTypes;
};

} // namespace typesystem

#endif // TYPESSTORE_H
