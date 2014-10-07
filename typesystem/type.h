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

#ifndef TYPE_H
#define TYPE_H

#include <memory>
#include <string>
#include <vector>

namespace typesystem {

class Type {
public:
    enum TypeClass {
        numeric = (1 << 5),
        boolean = (1 << 6),
        primitive = (1 << 7)
    };

    enum TypeId {
        // incomplete types
        Auto = -1,

        // Built in types
        Void = 0,
        Int = (1 | numeric | primitive),
        Bool = (boolean | primitive)
    };

    virtual std::string name() const = 0;
    virtual TypeId typeId() const = 0;
    virtual bool is(TypeClass type) const;

    virtual ~Type() {}
};

} // namespace typesystem

#endif
